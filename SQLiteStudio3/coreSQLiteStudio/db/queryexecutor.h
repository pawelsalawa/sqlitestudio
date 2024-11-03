#ifndef QUERYEXECUTOR_H
#define QUERYEXECUTOR_H

#include "db/db.h"
#include "coreSQLiteStudio_global.h"
#include "common/bistrhash.h"
#include "parser/ast/sqlitequery.h"
#include "parser/ast/sqlitequerytype.h"
#include "datatype.h"
#include <QObject>
#include <QHash>
#include <QMutex>
#include <QRunnable>

/** @file */

class Parser;
class QueryExecutorStep;
class DbPlugin;
class ChainExecutor;

/**
 * @brief Advanced SQL query execution handler.
 *
 * QueryExecutor is an advanced SQL query execution handler, which lets you execute any query (with subqueries, joins, etc)
 * and is capable of providing meta information about returned data, such as ROWID for all rows and columns,
 * data sources (database, table and column) for every column, rows affected, total rows number for query, etc.
 * All of this available for both SQLite versions: 2 and 3.
 *
 * Queries are executed asynchronously. To handle result a lambda function can be used (or any function pointer),
 * or manual tracking of asynchronous execution ID and signals from this class. Function pointers and lambdas
 * are recommended way to handle results.
 *
 * It also allows you to:
 * <ul>
 * <li>programatically define sorting on desired column.</li>
 * <li>define result rows paging (page size and queried page)</li>
 * <li>refer other databases by their symbolic name and they will be attached and detached on the fly</li>
 * <li>define maximum cell data size (in bytes), so you won't read too much data at once</li>
 * </ul>
 *
 * Total number of result rows is counted by a separate call to the database (using <tt>SELECT count(*) ...</tt>)
 * and its result is provided later, which is signalized by signal resultsCountingFinished(). Row counting can
 * be disabled with setSkipRowCounting(). See "Counting query" section below for details.
 *
 * The simplest use case would be:
 * @code
 * Db* db = getDb();
 * QueryExecutor *executor = new QueryExecutor(db, "SELECT * FROM table");
 * executor->exec([=](SqlQueryPtr results)
 * {
 *     if (results->isError())
 *     {
 *          qCritical() << "Error " << results->getErrorCode() << ": " << results->getErrorText() << "\n";
 *          return;
 *     }
 *     qDebug() << results->valueList();
 * }
 * @endcode
 *
 * Unless you want some of QueryExecutor's special features, it's recommended to use
 * Db::exec() and Db::asyncExec(), because while QueryExecutor is powerful, it also does lots of thing underneeth
 * you may not need at all.
 *
 * \note This class is used in SQL editor window (SqlQueryModel) to execute queries entered by the user.
 *
 * \section smart_simple_sec "smart mode" vs "simple mode"
 *
 * Documentation of this class references many times to "smart mode" and "simple mode" expressions.
 * The "smart mode" means that the QueryExecutor was able to parse the input query, modify it for its needs
 * (add some meta-information columns, etc) and executed modified query successfully.
 * When the "smart mode" fails (which should be rare), the "simple mode" steps in as a fallback strategy.
 * The "simple mode" doesn't modify input query, just directly executes in on the database
 * and then QueryExecutor tries to extract as much meta-information from "simple mode" as it can (which is not much).
 *
 * The "simple mode" also doesn't apply any paging (see QueryExecutor::setPage()), nor data size limits
 * (see QueryExecutor::setDataLengthLimit()).
 *
 * The meta-information is all the data from the query that is not the essential data requested in the input query.
 * That is full description on all requested columns (their source tables, databases, data types),
 * ROWID value for all returned data rows, and more...
 *
 * \section counting_query Counting query
 *
 * QueryExecutor can split results into pages. In such cases, results are not all read, instead they are limited
 * at SQL level with LIMIT and OFFSET keywords. Because of that it is impossible to tell how many rows
 * would actualy be returned if there were no limit keywords.
 *
 * To deal with it the QueryExecutor makes extra query execution, which happens asynchronously to the main query
 * execution. This extra execution starts just after the main query execution has finished (with success).
 * This extra query (aka "Counting query") is made of original query wrapped with:
 * @code
 * SELECT count(*) FROM (original_query)
 * @endcode
 * This way QueryExecutor know the true number of rows to be retuend by the query.
 *
 * Since this extra query execution takes some extra time, this is performed asynchronously and only after
 * successful execution of the main query. If you need to work with QueryExecutor::getTotalRowsReturned(),
 * wait for the QueryExecutor::resultsCountingFinished() signal first.
 *
 * Row counting query execution can be disabled with QueryExecutor::setSkipRowCounting(),
 */
class API_EXPORT QueryExecutor : public QObject, public QRunnable
{
    Q_OBJECT

    public:
        /**
         * @brief General reasons for which results data cannot be edited.
         */
        enum class EditionForbiddenReason
        {
            NOT_A_SELECT, /**< Executed query was not a SELECT. Only SELECT results can be edited. */
            SMART_EXECUTION_FAILED /**<
                                    * QueryExecutor could not perform "smart" execution,
                                    * which means that it was unable to gather meta information
                                    * about returned data and therefore it cannot tell what are ROWIDs
                                    * or data sources for each column. Still it was able to perform
                                    * simple (direct, without query modifications) execution
                                    * and it returned results, so they can be presented to the user,
                                    * but not edited.
                                    *
                                    * This happens usually when there's a but in SQLiteStudio,
                                    * which caused - for example - error during query parsing by Parser,
                                    * or other query syntax issues, that wasn't handled correctly
                                    * by SQLiteStudio.
                                    */
        };

        /**
         * @brief Per-column reasons for which the data in the column cannot be edited.
         */
        enum class ColumnEditionForbiddenReason
        {
            COMPOUND_SELECT, /**<
                              * The data cell comes from compound SELECT (UNION, EXCEPT, INTERSECT),
                              * which makes it problematic to SQLiteStudio to find out to which table
                              * does the particular row belong to.
                              *
                              * It might be resolved in future SQLiteStudio versions and this enum value
                              * would disappear then.
                              */
            GROUPED_RESULTS, /**<
                              * The data cell comes from SELECT with aggregated results, therefore it's
                              * hard to hard what were ROWIDs of each row in the results.
                              *
                              * It might be resolved in future SQLiteStudio versions and this enum value
                              * would disappear then.
                              */
            DISTINCT_RESULTS, /**<
                               * The data cell comes from SELECT DISTINCT clause, therefore extracting
                               * ROWIDs from the results is impossible, becasuse querying ROWID would
                               * make every row unique, therefore DISTINCT would not remove any rows,
                               * even the rest of the data (which matters to the user) would not be
                               * unique and should have been removed by the DISTINCT keyword.
                               *
                               * Because of that, SQLiteStudio doesn't extract ROWIDs for DISTINCT
                               * queries, so the results are accurate, but in consequence,
                               * the data cannot be edited.
                               */
            EXPRESSION,       /**<
                               * The data cell is a result of a formula, function or other expression,
                               * which is not a direct data source, therefore it's impossible to change
                               * it's value.
                               */
            SYSTEM_TABLE,     /**<
                               * The data cell comes from system table (sqlite_*) and those tables cannot
                               * be edited.
                               */
            COMM_TAB_EXPR,    /**<
                               * The data cell comes from system "WITH common-table-expression" SELECT
                               * statement and those tables cannot be edited for the same reasons as
                               * in COMPOUND_SELECT case. To learn about common table expression statement,
                               * see http://sqlite.org/lang_with.html
                               */
            VIEW_NOT_EXPANDED,/**<
                               * The data cell comes from a VIEW that was not expanded (because there were
                               * multi-level views), therefore it was impossible to get ROWID for the cell.
                               */
        };

        /**
         * @brief Sort order definition.
         *
         * QueryExecutor supports programmatic sort order definition.
         * It supports smooth transition from/to Qt sorting direction enum
         * and defines sorting column by its index (0-based).
         */
        struct API_EXPORT Sort
        {
            /**
             * @brief Sorting order.
             */
            enum Order
            {
                ASC, /**< Ascending order */
                DESC, /**< Descending order */
                NONE /**< No sorting at all */
            };

            /**
             * @brief Default constructor with no sorting defined.
             *
             * Constructed object uses NONE as sorting order.
             */
            Sort();

            /**
             * @brief Creates sort order with given order on given column.
             * @param order Order to sort with.
             * @param column 0-based column number.
             */
            Sort(Order order, int column);

            /**
             * @brief Creates sort order with given order on given column.
             * @param order Qt typed sort order (Qt::AscendingOrder, Qt::DescendingOrder).
             * @param column 0-based column number.
             */
            Sort(Qt::SortOrder order, int column);

            /**
             * @brief Gets Qt typed sort order.
             * @return Sort order.
             */
            Qt::SortOrder getQtOrder() const;

            /**
             * @brief Sorting order.
             */
            Order order = NONE;

            /**
             * @brief 0-based column number to sort by.
             */
            int column = -1;
        };

        typedef QList<Sort> SortList;

        /**
         * @brief ResultColumn as represented by QueryExecutor.
         *
         * QueryExecutor has its own result column representation, because it provides more
         * meta information on the column.
         */
        struct API_EXPORT ResultColumn
        {
            /**
             * @brief Database name that the result column comes from.
             *
             * It's an SQLite internal name of the database, which means it's either "main", or "temp",
             * or symbolic name of registered database (as represented in the databases tree),
             * or the name of any attached databases.
             *
             * Symbolic database name is provided when user used it in his query and SQLiteStudio attached
             * it transparently. In that case the temporary name used for "ATTACH" statement would make no sense,
             * because that database was detached automatically after the query execution finished.
             *
             * In case of databases attached manually by user, it's exactly the same string as used when executing
             * "ATTACH" statement.
             */
            QString database;

            /**
             * @brief Table name that the result column comes from.
             */
            QString table;

            /**
             * @brief Table column name that the result column comes from.
             */
            QString column;

            /**
             * @brief Alias defined for the result column in the query.
             */
            QString alias;

            /**
             * @brief Table alias defined in the query.
             *
             * This is an alias defined in the query for the table that the result column comes from.
             */
            QString tableAlias;

            /**
             * @brief Name of the column as presented to user.
             *
             * This is the name of a column as SQLite would present it to the user.
             * If the query requested just a column from table, it will be that column name.
             * If the query resuested two columns with the same name, then the second column will get
             * suffix ":1", next one would get suffix ":2", and so on.
             * For expressions the display name is direct copy of the SQL code used to define the expression.
             *
             * If the alias was defined in query, than it's used for the display name instead of anything else.
             */
            QString displayName;

            /**
             * @brief QueryExecutor's internal alias for the column.
             *
             * This value has no sense outside of QueryExecutor. It's used by QueryExecutor to
             * keep track of columns from subselects, etc.
             */
            QString queryExecutorAlias;

            /**
             * @brief Set of reasons for which column editing is denied.
             *
             * If the set is empty, it means that the column can be edited.
             */
            QSet<ColumnEditionForbiddenReason> editionForbiddenReasons;

            /**
             * @brief Flag indicating that the column is actually an expression.
             *
             * Column representing an expression is not just a column and it should not be ever wrapped with
             * quoting wrapper ([], "", ``). Such a column is for example call to the SQL function.
             *
             * For regular columns this will be false.
             */
            bool expression = false;
        };

        /**
         * @brief Shared pointer to ResultColumn.
         */
        typedef QSharedPointer<ResultColumn> ResultColumnPtr;

        /**
         * @brief Combined row ID columns for tables in the query.
         *
         * Since version 3.8.2 SQLite introduced the "WITHOUT ROWID" clause. It allows tables to have no
         * ROWID built-in. Such tables must have PRIMARY KEY defined, which does the job of the unique key
         * for the table.
         *
         * This structure describes the unique key for the table, regardless if it's a regular ROWID,
         * or if it's a PRIMARY KEY on a column, or if it's a multi-column PRIMARY KEY.
         *
         * You should always understand it as a set of PRIMARY KEY columns for given table.
         * Referencing to that table using given columns will guarantee uniqueness of the row.
         *
         * In case of regular table (with no "WITHOUT ROWID" clause), there will be only one column
         * defined in ResultRowIdColumn::columns and it will be named "ROWID".
         */
        struct API_EXPORT ResultRowIdColumn
        {
            /**
             * @brief Database name that the table with this row ID is in.
             *
             * It's the actual database name as SQLite sees it. That means it is a "main", or any attach name.
             */
            QString database;

            /**
             * @brief Symbolic database name as listed in databases list.
             *
             * It can be empty if database was not explicitly passed in the query.
             */
            QString dbName;

            /**
             * @brief Table name that the row ID is for.
             */
            QString table;

            /**
             * @brief Table alias defined in the query.
             * @see ResultColumn::tableAlias
             */
            QString tableAlias;

            /**
             * @brief Mapping from alias to real column.
             *
             * This is mapping from QueryExecutor's internal aliases for columns
             * into primary key column names of the table that the result column comes from.
             *
             * If you want to get list of column names used for RowId, use values() on this member.
             * If you want to get list of query executor aliases, use keys() on this member.
             */
            QHash<QString,QString> queryExecutorAliasToColumn;
        };

        /**
         * @brief Shared pointer to ResultRowIdColumn.
         */
        typedef QSharedPointer<ResultRowIdColumn> ResultRowIdColumnPtr;

        /**
         * @brief Table that was a data source for at least one column in the query.
         */
        struct API_EXPORT SourceTable
        {
            /**
             * @brief Table's database.
             *
             * Same rules apply as for ResultColumn::database.
             */
            QString database;

            /**
             * @brief Table name.
             */
            QString table;

            /**
             * @brief Table alias defined in query.
             */
            QString alias;
        };

        /**
         * @brief Shared pointer to SourceTable.
         */
        typedef QSharedPointer<SourceTable> SourceTablePtr;

        /**
         * @brief Query execution context.
         *
         * This class is used to share data across all executor steps.
         * It also provides initial configuration for executor steps.
         */
        struct Context
        {
            /**
             * @brief Query string after last query step processing.
             *
             * Before any step was executed, this is the same as originalQuery.
             *
             * The processed query is the one that will be executed in the end,
             * so any steps should apply their changes to this query.
             *
             * This string should be modified and updated from QueryExecutorStep implementations.
             *
             * You won't usually modify this string directly. Instead you will
             * want to use one of 2 methods:
             * <ul>
             * <li>Modify tokens</li> - modify tokens of top level objects in parsedQueries
             * and call QueryExecutorStep::updateQueries().
             * <li>Modify parsed objects</li> - modify logical structure and values of
             * objects in parsedQueries, then call on those objects SqliteStatement::rebuildTokens()
             * and finally call QueryExecutorStep::updateQueries.
             * </ul>
             *
             * The parsedQueries are refreshed every time when QueryExecutor executes
             * QueryExecutorParse step.
             */
            QString processedQuery;

            /**
             * @brief Number of milliseconds that query execution took.
             *
             * This is measured and set by QueryExecutorStepExecute step.
             */
            qint64 executionTime = 0;

            /**
             * @brief Number of rows affected by the query.
             */
            qint64 rowsAffected = 0;

            /**
             * @brief Total number of rows returned from query.
             *
             * It provides correct number for all queries, no matter if it's SELECT, PRAGMA, or other.
             */
            qint64 totalRowsReturned = 0;

            /**
             * @brief Total number of pages.
             *
             * If there's a lot of result rows, they are split to pages.
             * There's always at least one page of results.
             */
            int totalPages = 1;

            /**
             * @brief Defines if row counting will be performed.
             *
             * In case of EXPLAIN or PRAGMA queries the number of result rows is not provided from
             * SQLite (at least not from Qt's drivers for them). Instead we need to manually count
             * number of rows. This is when this flag is set (it's done by QueryExecutor,
             * no need to care about it).
             */
            bool rowsCountingRequired = false;

            /**
             * @brief Executing query in EXPLAIN mode.
             *
             * This is configuration parameter passed from QueryExecutor just before executing
             * the query. It can be defined by QueryExecutor::setExplainMode().
             */
            bool explainMode = false;

            /**
             * @brief Defines if row counting should be skipped.
             *
             * This is a configuration flag predefined by QueryExecutor just before executing starts.
             * You can set it with QueryExecutor::setSkipRowCounting().
             *
             * Row counting is done asynchronously, just after normal query execution is finished.
             * It's done by executing yet another query, which is more or less an orginal query
             * wrapped with "SELECT count(*) FROM (...)".
             *
             * Separate counting has to be done, because QueryExecutor adds LIMIT and OFFSET
             * to SELECT queries for results paging.
             *
             * When counting is done, the resultsCountingFinished() signal is emitted.
             */
            bool skipRowCounting = false;

            /**
             * @brief Parameters for query execution.
             *
             * It's defined by setParam().
             */
            QHash<QString,QVariant> queryParameters;

            /**
             * @brief Results handler function pointer.
             *
             * This serves the same purpose as in Db class. It's used for execution
             * with results handled by provided function. See Db::QueryResultsHandler for details.
             *
             * It's defined by exec().
             */
            Db::QueryResultsHandler resultsHandler = nullptr;

            /**
             * @brief List of queries parsed from input query string.
             *
             * List of parsed queries is updated each time the QueryExecutorParseQuery step
             * is executed. When it's called is defined by QueryExecutor::executionChain.
             */
            QList<SqliteQueryPtr> parsedQueries;

            /**
             * @brief Results of executed query.
             *
             * This is results object defined by the final query execution. It means that the
             * query executed passed all preprocessing steps and was executed in its final form.
             *
             * This member is defined by QueryExecutorExecute step.
             */
            SqlQueryPtr executionResults;

            /**
             * @brief Currently attached databases.
             *
             * This is a cross-context information about currently attached databases.
             * As QueryExecutorAttaches step does attaching, other steps may need information
             * about attached databases. It's a map of orginal_db_name_used to attached_name.
             */
            BiStrHash dbNameToAttach;

            /**
             * @brief Sequence used by executor steps to generate column names.
             */
            int colNameSeq = 0;

            /**
             * @brief List of reasons that editing results is forbidden for.
             *
             * Executor steps may decide that the results of query cannot be edited.
             * In that case they add proper enum to this set.
             */
            QSet<EditionForbiddenReason> editionForbiddenReasons;

            /**
             * @brief Result columns that provide ROWID.
             *
             * QueryExecutorAddRowIds step adds those columns. There is one or more columns
             * per data source table mentioned in the query. It depends on "WITHOUT ROWID" clause
             * in CREATE TABLE of the source table.
             */
            QList<ResultRowIdColumnPtr> rowIdColumns;

            /**
             * @brief Result columns from the query.
             *
             * List of result columns, just like they would be returned from regular execution
             * of the query. Column in this list are not just a names of those columns,
             * they provide full meta information about every single column.
             */
            QList<ResultColumnPtr> resultColumns;

            /**
             * @brief Data source tables mentioned in the query.
             *
             * List of tables used as data source in the query.
             */
            QSet<SourceTablePtr> sourceTables;

            /**
             * @brief Map of column aliases containing column types.
             *
             * Keys are query executor column aliases of column representing types
             * and values are query executor column aliases of column to which these types apply to.
             */
            QHash<QString, QString> typeColumnToResultColumnAlias;

            /**
             * @brief Query used for counting results.
             *
             * Filled with SQL to be used for results counting (even if counting is disabled).
             * @see QueryExecutor::getCountingQuery()
             */
            QString countingQuery;

            /**
             * @brief Flag indicating results preloading.
             *
             * Causes flag Db::Flag::PRELOAD to be added to the query execution.
             */
            bool preloadResults = false;

            /**
             * @brief Tells if executed queries did modify database schema.
             *
             * This is defined by QueryExecutorDetectSchemaAlter step
             * and can be accessed by QueryExecutor::wasSchemaModified().
             */
            bool schemaModified = false;

            /**
             * @brief Tells if executed query was one of DELETE, UPDATE or INSERT.
             */
            bool dataModifyingQuery = false;

            /**
             * @brief Forbids QueryExecutor to return meta columns.
             *
             * See QueryExecutor::noMetaColumns for details.
             */
            bool noMetaColumns = false;

            /**
             * @brief Contains error code from smart execution.
             *
             * This is always set by the smart execution method. It's useful if the smart execution
             * failed and the simple execution method is being performed. In that case the query
             * result will contain error code from simple execution, hiding the original error code
             * from smart execution.
             */
            int errorCodeFromSmartExecution = 0;

            /**
             * @brief Contains error message from smart execution.
             *
             * This is always set by the smart execution method. It's useful if the smart execution
             * failed and the simple execution method is being performed. In that case the query
             * result will contain error message from simple execution, hiding the original error
             * message from smart execution.
             */
            QString errorMessageFromSmartExecution;

            /**
             * @brief Flag indicating whether views were replaced/expanded.
             *
             * In other words, this flag tells whether the ReplaceViews step of query executor
             * was executed, or skipped (due to many levels of views). False = skipped.
             */
            bool viewsExpanded = false;
        };

        /**
         * @brief Position for custom executor steps.
         *
         * These position define where in query executor chain of steps the new, registered step will be placed.
         * It's used in arguments of registerStep() method.
         *
         * If multiple steps are registered at same position, they will be executed in order they were registered.
         *
         * Values of positions determin what actions have been already taken by the executor, so for example
         * AFTER_ATTACHES means, that executor already attached referenced databases and replaced occurrences of objects
         * from that databases.
         *
         * Enumerations are in order as the steps are executed.
         *
         * If you need more detailed description about certain steps performed by query executor, see documentation
         * of their classes - all these classes name start with QueryExecutor prefix.
         */
        enum StepPosition {
            FIRST,                      /**< As first step */
            AFTER_ATTACHES,             /**< After transparent attaching is applied (detected tables are defined to be attached first). */
            AFTER_REPLACED_VIEWS,       /**< After referenced views have been replaced with subqueries */
            AFTER_ROW_IDS,              /**< After ROWID columns have been added to result columns */
            AFTER_REPLACED_COLUMNS,     /**< After all columns have been explicitly listed in result list, together with unique alias names */
            AFTER_ORDER,                /**< After order clause was applied/modified */
            AFTER_DISTINCT_WRAP,        /**< After wrapping SELECT was added in case of DISTINCT or GROUP BY clauses were used */
            AFTER_COLUMN_TYPES,         /**< After typeof() result meta columns were added */
            AFTER_COLUMN_FILTERS,       /**< After WHERE filters applied */
            AFTER_ROW_LIMIT_AND_OFFSET, /**< After LIMIT and ORDER clauses were added/modified. This is the last possible moment, directly ahead of final query execution */
            JUST_BEFORE_EXECUTION,      /**< Same as AFTER_ROW_LIMIT_AND_OFFSET */
            LAST                        /**< Same as AFTER_ROW_LIMIT_AND_OFFSET */
        };

        /**
         * @brief Interface for producing query executor steps.
         *
         * It can be used for overloaded version of registerStep() method,
         * in case a step is stateful and needs to be created/deleted for every query executor processing.
         *
         * If step is stateless, it can be registered directly as an instance, using the other version of registerStep() method.
         *
         * This is an abstract class and not pointer to function, because it has to be comparable (and std::function is not),
         * so it's possible to deregister the factory later on.
         */
        class StepFactory {
            public:
                virtual QueryExecutorStep* produceQueryExecutorStep() = 0;
        };

        /**
         * @brief Creates query executor, initializes internal context object.
         * @param db Optional database. If not provided, it has to be defined later with setDb().
         * @param query Optional query to execute. If not provided, it has to be defined later with setQuery().
         * @param parent Parent QObject.
         */
        QueryExecutor(Db* db = nullptr, const QString& query = QString(), QObject *parent = 0);

        /**
         * @brief Releases internal resources.
         */
        ~QueryExecutor();

        /**
         * @brief Defined query to be executed.
         * @param query SQL query string.
         *
         * The query string can actually be multiple queries separated with a semicolon, just like you would
         * write multiple queries in the SQL Editor window. Query executor will handle that.
         *
         * The query can contain parameter placeholders (such as :param, \@param). To bind values to params
         * use setParam().
         */
        void setQuery(const QString& query);

        /**
         * @brief Executes the query.
         * @param resultsHandler Optional handler function pointer, can be lambda function. See Db::QueryResultsHandler for details.
         *
         * While execution is asynchronous, the executor notifies about results by signals.
         * In case of success emits executionFinished(), in case of error emits executionFailed().
         */
        void exec(Db::QueryResultsHandler resultsHandler = nullptr);

        /**
         * @brief Interrupts current execution.
         *
         * Calls Db::asyncInterrupt() internally.
         */
        void interrupt();

        /**
         * @brief Executes counting query.
         * @return true if counting query is executed (in async mode) or was executed correctly (in sync mode), false on error.
         *
         * Executes (asynchronously) counting query for currently defined query. After execution is done, the resultsCountingFinished()
         * signal is emitted.
         *
         * Counting query is made of original query wrapped with "SELECT count(*) FROM (original_query)".
         *
         * It is executed after the main query execution has finished.
         *
         * If query is being executed in async mode, the true result (sucess/fail) will be known from later, not from this method.
         */
        bool countResults();

        /**
         * @brief Gets time of how long it took to execute query.
         * @return Execution time in milliseconds.
         *
         * The execution time is number of milliseconds from begining of the query execution, till receiving of the results.
         */
        qint64 getLastExecutionTime() const;

        /**
         * @brief Gets number of rows affected by the query.
         * @return Affected rows number.
         *
         * Rows affected are defined by DbPlugin implementation and are usually a number of rows modified by UPDATE statement,
         * or deleted by DELETE statement, or inserted by INSERT statement.
         */
        qint64 getRowsAffected() const;

        /**
         * @brief Gets number of rows returned by the query.
         * @return Number of rows.
         *
         * If QueryExecutor limits result rows number (if defined by setResultsPerPage()), the actual number of rows
         * to be returned from query can be larger. This methods returns this true number of rows,
         * that would be returned from the query.
         *
         * Calling this method makes sense only after resultsCountingFinished() was emitted, otherwise the value
         * returned will not be accurate.
         */
        qint64 getTotalRowsReturned() const;

        /**
         * @brief Gets type of the SQL statement in the defined query.
         * @param index Index of the SQL statement in the query (statements are separated by semicolon character), or -1 to get the last one.
         * @return Type of the query. If there were no parsed queries in the context, or if passed index is out of range,
         * then SqliteQueryType::UNDEFINED is returned.
         */
        SqliteQueryType getExecutedQueryType(int index = -1);

        /**
         * @brief Provides set of data source tables used in query.
         * @return Set of tables.
         */
        QSet<QueryExecutor::SourceTablePtr> getSourceTables() const;

        /**
         * @brief Gets number of pages available.
         * @return Number of pages.
         *
         * Since QueryExecutor organizes results of the query into pages, this method gives number of pages that is necessary
         * to display all the data. In other words: "results of this method" - 1 = "last page index".
         *
         * Single page size is defined by setResultsPerPage().
         */
        int getTotalPages() const;

        /**
         * @brief Gets ordered list of result columns.
         * @return Result columns.
         *
         * See Context::resultColumns for details.
         */
        QList<QueryExecutor::ResultColumnPtr> getResultColumns() const;

        /**
         * @brief Gets map of meta columns providing SQLite data types.
         * @return Map of type column alias to target column alias (for which the type applies).
         */
        QHash<QString, QString> getTypeColumns() const;

        /**
         * @brief Gets list of ROWID columns.
         * @return ROWID columns.
         *
         * Note, that this returns list of ROWID columns as entities. This means that for ROWID a single ROWID column
         * can be actually couple of columns in the results. To count the ROWID columns offset for extracting
         * data columns use getMetaColumnCount().
         *
         * See Context::rowIdColumns for details.
         */
        QList<QueryExecutor::ResultRowIdColumnPtr> getRowIdResultColumns() const;

        /**
         * @brief Gives number of meta columns in the executed query.
         * @return Number of the actual meta columns (such as ROWID columns) added to the executed query.
         *
         * This method should be used to find out the number of meta columns that were added to the begining
         * of the result columns in the executed query. This way you can learn which column index use as a start
         * for reading the actual data from the query.
         *
         * Meta columns are used by QueryExecutor to find more information about the query being executed
         * (like ROWID of each row for example).
         */
        int getMetaColumnCount() const;

        /**
         * @brief Gets reasons for which editing results is forbidden.
         * @return Set of reasons.
         *
         * See Context::editionForbiddenReasons for details.
         */
        QSet<EditionForbiddenReason> getEditionForbiddenGlobalReasons() const;

        /**
         * @brief Defines named bind parameter for the query.
         * @param name Name of the parameter (without the : or @ prefix).
         * @param value Value of the parameter.
         *
         * Positional (index oriented) parameters are not supported by the QueryExecutor.
         * Always use named parameters with QueryExecutor.
         */
        void setParam(const QString& name, const QVariant& value);

        /**
         * @brief Assigns all query parameters at once.
         * @param params All bind parameters for the query.
         *
         * This is same as setParam(), except it overrides all current params with given ones.
         * It allows for setting all params at once.
         */
        void setParams(const QHash<QString, QVariant>& params);

        /**
         * @brief Gets currently defined database.
         * @return Database object, or null pointer if not yet defined.
         */
        Db* getDb() const;

        /**
         * @brief Defines new database for query execution.
         * @param value Database object. It should be open before calling exec().
         */
        void setDb(Db* value);

        /**
         * @brief Gets original, not modified query.
         * @return SQL query string.
         */
        QString getOriginalQuery() const;

        /**
         * @brief Gets data size limit.
         * @return Number of bytes, or UTF-8/UTF-16 characters.
         *
         * See setDataLengthLimit() for details.
         */
        int getDataLengthLimit() const;

        /**
         * @brief Defines data size limit for results.
         * @param value Number of bytes, or UTF-8/UTF-16 characters.
         *
         * Limit is not defined by default and in that case it's not applied
         * to the query. To enable limit, set it to any positive number.
         * To disable limit, set it to any negative number.
         *
         * When QueryExecutor prepares query for execution, it applies SUBSTR()
         * to all result columns, so if the database has a huge value in some column,
         * SQLiteStudio won't load 1000 rows with huge values - that would kill performance
         * of the application. Instead it loads small chunk of every value.
         *
         * SqlQueryModel loads limited chunks of data and loads on-the-fly full cell values
         * when user requests it (edits the cell, or views it in form editor).
         *
         * Parameter defined by this method is passed to SQLite's SUBSTR() function.
         * As SQLite's documentation stand, numbers passed to that function are treated
         * as number of bytes for non-textual data and for textual data they are number
         * of characters (for UTF-8 and UTF-16 they can be made of more than 1 byte).
         */
        void setDataLengthLimit(int value);

        // TODO manual row counting -> should be done by query executor already and returned in total rows
        /**
         * @brief Tests if manual row counting is required.
         * @return True if manual counting is required.
         *
         * In case of some queries the getTotalRowsReturned() won't provide proper value.
         * Then you will need to count result rows from the results object.
         *
         * It's okay, because this applies only for EXPLAIN and PRAGMA queries,
         * which will never return any huge row counts.
         */
        bool isRowCountingRequired() const;

        /**
         * @brief Gets SQL query used for counting results.
         * @return SQL query.
         *
         * This is the query used by countResults().
         */
        QString getCountingQuery() const;

        /**
         * @brief Gets number of rows per page.
         * @return Number of rows.
         *
         * By default results are not split to pages and this method will return -1.
         */
        int getResultsPerPage() const;

        /**
         * @brief Defines number of rows per page.
         * @param value Number of rows.
         *
         * By default results are not split to pages.
         * See setPage() for details on enabling and disabling paging.
         */
        void setResultsPerPage(int value);

        /**
         * @brief Gets current results page.
         * @return Page index.
         *
         * Results page is 0-based index. It's always value between 0 and (getTotalPages() - 1).
         * If results paging is disabled (see setResultsPerPage()), then this method
         * will always return 0, as this is the first page (and in that case - the only one).
         */
        int getPage() const;

        /**
         * @brief Defines results page for next execution.
         * @param value 0-based page index.
         *
         * If page value is negative, then paging is disabled.
         * Any positive value or 0 enables paging and sets requested page of results to given page.
         *
         * If requested page value is greater than "getTotalPages() - 1", then no results will be returned.
         * It's an invalid page value.
         * If requested page value is lower then 0, then paging is disabled.
         *
         * Once the page is defined, the exec() must be called to get results
         * from new defined page.
         */
        void setPage(int value);

        /**
         * @brief Tests if there's any execution in progress at the moment.
         * @return true if the execution is in progress, or false otherwise.
         */
        bool isExecutionInProgress();

        /**
         * @brief Gets sorting defined for executor.
         * @return Sorting definition.
         *
         * See Sort for details.
         */
        QueryExecutor::SortList getSortOrder() const;

        /**
         * @brief Defines sorting for next query execution.
         * @param value Sorting definition.
         *
         * Once the sorting definition is changed, the exec() must be called
         * to receive results in new order.
         */
        void setSortOrder(const QueryExecutor::SortList& value);

        /**
         * @brief Tests if row counting is disabled.
         * @return true if row counting will be skipped, or false otherwise.
         *
         * See Context::skipRowCounting for details.
         */
        bool getSkipRowCounting() const;

        /**
         * @brief Defines if executor should skip row counting.
         * @param value New value for this parameter.
         *
         * See Context::skipRowCounting for details.
         */
        void setSkipRowCounting(bool value);

        /**
         * @brief Asynchronous executor processing in thread.
         *
         * This is an implementation of QRunnable::run(), so the QueryExecutor
         * does it's own asynchronous work on object members.
         */
        void run();

        /**
         * @brief Tests if query execution should be performed in EXPLAIN mode.
         * @return true if the mode is enabled, or false otherwise.
         */
        bool getExplainMode() const;

        /**
         * @brief Defines EXPLAIN mode for next query execution.
         * @param value true to enable EXPLAIN mode, or false to disable it.
         *
         * EXPLAIN mode means simply that the EXPLAIN keyword will be prepended
         * to the query, except when the query already started with the EXPLAIN keyword.
         *
         * Once the mode is changed, the exec() must be called
         * to receive "explain" results.
         */
        void setExplainMode(bool value);

        /**
         * @brief Defines results preloading.
         * @param value true to preload results.
         *
         * Results preloading is disabled by default. See Db::Flag::PRELOAD for details.
         */
        void setPreloadResults(bool value);

        bool getAsyncMode() const;
        void setAsyncMode(bool value);

        SqlQueryPtr getResults() const;
        bool wasSchemaModified() const;
        bool wasDataModifyingQuery() const;

        static QList<DataType> resolveColumnTypes(Db* db, QList<ResultColumnPtr>& columns, bool noDbLocking = false);

        bool getNoMetaColumns() const;
        void setNoMetaColumns(bool value);

        void setFilters(const QString& newFilters);
        QString getFilters() const;

        void handleErrorsFromSmartAndSimpleMethods(SqlQueryPtr results);

        /**
         * @brief Clears results handle and detaches any attached databases.
         */
        void releaseResultsAndCleanup();

        const QStringList& getRequiredDbAttaches() const;

        bool getForceSimpleMode() const;
        void setForceSimpleMode(bool value);

        bool isInterrupted() const;

        int getQueryCountLimitForSmartMode() const;
        void setQueryCountLimitForSmartMode(int value);

        /**
         * @brief Adds new step to the chain of execution
         * @param position Where in chain should the step be placed.
         * @param step Step implementation instance.
         *
         * If multiple steps are registered for the same position, they will be executed in the order they were registered.
         *
         * If step is registered with a plugin, remember to deregister the step upon plugin unload.
         * Best place for that is in Plugin::deinit().
         */
        static void registerStep(StepPosition position, QueryExecutorStep* step);

        /**
         * @brief Adds new step to chain of execution
         * @param position Where in chain should the step be placed.
         * @param stepFactory Factory for creating instance of the step.
         *
         * This is overloaded method for cases when the step is stateful and needs to be recreated for every invokation of query executor.
         */
        static void registerStep(StepPosition position, StepFactory* stepFactory);

        /**
         * @brief Removes extra step from the execution chain.
         * @param position Position from which the step should be removed.
         * @param step Step implementation instance.
         *
         * Removes step from list of additional steps to be performed. If such step was not on the list, this method does nothing.
         *
         * There is a position parameter to this method, becuase a step could be added to multiple different positions
         * and you decide from which you want to deregister it.
         */
        static void deregisterStep(StepPosition position, QueryExecutorStep* step);

        /**
         * @brief Removes extra step from the execution chain.
         * @param position Position from which the step should be removed.
         * @param stepFactory Factory to deregister.
         *
         * This is overloaded method to remove factory instead of instance of a step. To be used together with registerStep()
         * that also takes factory in parameters.
         */
        static void deregisterStep(StepPosition position, StepFactory* stepFactory);

    private:
        /**
         * @brief Executes query.
         *
         * It's called from run(). This is the execution of query but called from different
         * thread than exec() was called from.
         */
        void execInternal();

        /**
         * @brief Raises execution error.
         * @param code Error code. Can be either from SQLite error codes, or from SqlErrorCode.
         * @param text Error message.
         *
         * This method is called when some of executor's preconditions has failed, or when SQLite
         * execution raised an error.
         */
        void error(int code, const QString& text);

        /**
         * @brief Build chain of executor steps.
         *
         * Defines executionChain by adding new QueryExecutorStep descendants.
         * Each step has its own purpose described in its class documentation.
         * See inheritance hierarchy of QueryExecutorStep.
         */
        void setupExecutionChain();

        /**
         * @brief Deletes executor step objects.
         *
         * Deletes all QueryExecutorStep objects from executionChain clears the list.
         */
        void clearChain();

        /**
         * @brief Executes all steps from executor chain.
         *
         * The steps chain is defined by setupExecutionChain().
         * On execution error, the stepFailed() is called and the method returns.
         */
        void executeChain();

        /**
         * @brief Executes the original, unmodified query.
         *
         * When smart execution (using steps chain) failed, then this method
         * is a fallback. It executes original query passed to the executor.
         * Given, that query was not modified, it cannot provide meta information,
         * therefore results of that execution won't editable.
         */
        void executeSimpleMethod();

        /**
         * @brief Tests whether the original query is a SELECT statement.
         * @return true if the query is SELECT, or false otherwise.
         *
         * This method assumes that there was a problem with parsing the query with the Parser
         * (and that's why we're using simple execution method) and so it tries to figure out
         * a query type using other algorithms.
         */
        bool simpleExecIsSelect();

        /**
         * @brief Releases resources acquired during query execution.
         *
         * Currently it just detaches databases attached for query execution needs (transparent
         * database attaching feature).
         */
        void cleanup();

        /**
         * @brief Extracts counting query results.
         * @param asyncId Asynchronous ID of the counting query execution.
         * @param results Results from the counting query execution.
         * @return true if passed asyncId is the one for currently running counting query, or false otherwise.
         *
         * It's called from database asynchronous execution thread. The database might have executed
         * some other acynchronous queries too, so this method checks if the asyncId is the expected one.
         *
         * Basicly this method is called a result of countResults() call. Extracts counted number of rows
         * and stores it in query executor's context.
         */
        bool handleRowCountingResults(quint32 asyncId, SqlQueryPtr results);

        QStringList applyFiltersAndLimitAndOrderForSimpleMethod(const QStringList &queries);

        /**
         * @brief Creates instances of steps for all registered factories for given position.
         * @param position Position for which factories will be used.
         * @return List of instances of steps from given factories.
         */
        QList<QueryExecutorStep*> createSteps(StepPosition position);

        /**
         * @brief Query executor context object.
         *
         * Context object is shared across all execution steps. It's (re)initialized for every
         * call to exec(). Initialization involves copying configuration parameters (such as sortOrder,
         * explainMode, etc) from local members to the context.
         *
         * During steps execution the context is used to share information between steps.
         * For example if one step modifies query in anyway, it should store updated query
         * in Context::processedQuery. See QueryExecutorStep for details on possible methods
         * for updating Context::processedQuery (you don't always have to build the whole processed
         * query string by yourself).
         *
         * Finally, the context serves as a results container from all steps. QueryExecutor reads
         * result columns metadata, total rows number, affected rows and other information from the context.
         */
        Context* context = nullptr;

        /**
         * @brief Database that all queries will be executed on.
         *
         * It can be passed in constructor or defined later with setDb(), but it cannot be null
         * when calling exec(). The exec() will simply return with no execution performed
         * and will log a warning.
         */
        Db* db = nullptr;

        /**
         * @brief Synchronization mutex for "execution in progress" state of executor.
         *
         * The state of "execution in progress" is the only value synchronized between threads.
         * It makes sure that single QueryExecutor will execute only one query at the time.
         */
        mutable QMutex executionMutex;
        mutable QMutex interruptionMutex;

        /**
         * @brief Query to execute as defined by the user.
         *
         * This is a copy of original query provided by user to the executor.
         */
        QString originalQuery;

        QStringList queriesForSimpleExecution;

        /**
         * @brief Predefined number of results per page.
         *
         * See setResultsPerPage() for details.
         */
        int resultsPerPage = -1;

        /**
         * @brief Predefined results page index.
         *
         * See setPage() for details.
         */
        int page = -1;

        /**
         * @brief Predefined sorting order.
         *
         * There's no sorting predefined by default. If you want it, you have to apply it with setSortOrder().
         */
        SortList sortOrder;

        /**
         * @brief Flag indicating that the execution is currently in progress.
         *
         * This variable is synchronized across threads and therefore you can always ask QueryExecutor
         * if it's currently busy (with isExecutionInProgress()).
         */
        bool executionInProgress = false;

        /**
         * @brief Flag indicating that the most recent execution was made in "simple mode".
         *
         * This flag is set by executeSimpleMethod() method. See its documentation for details.
         * The exec() resets this flag to false each time, but each time the smart execution fails,
         * the executeSimpleMethod() is called and the flag is set to true.
         */
        bool simpleExecution = false;

        /**
         * @brief Flag indicating that the most recent execution was interrupted.
         *
         * This flag is set only if execution was interrupted by call to interrupt() on this class.
         * If the execution was interrupted by another thread (which called sqlite_interrupt()
         * or Db::interrupt()), then this flag is not set.
         *
         * This variable is tested at several stages of query execution in order to abort
         * execution if the interruption was already requested.
         */
        bool interrupted = false;

        /**
         * @brief Flag indicating that the execution is performed in EXPLAIN mode.
         *
         * See setExplainMode() for details.
         */
        bool explainMode = false;

        /**
         * @brief Flag indicating that the row counting was disabled.
         *
         * See Context::skipRowCounting for details.
         */
        bool skipRowCounting = false;

        /**
         * @brief Defines results data size limit.
         *
         * See setDataLengthLimit() for details.
         */
        int dataLengthLimit = -1;

        /**
         * @brief Optional filters to apply to the query.
         * If not empty, it will be appended to the WHERE clause at the very end of execution chain,
         * skipping complex result colum analysis, etc.
         */
        QString filters;

        /**
         * @brief Limit of queries, after which simple mode is used.
         *
         * Up to the defined limit the smart execution will be used (unless #forceSimpleMode was set).
         * After exceeding this limit, the simple mode will be used.
         * Set to negative number to disable this limit.
         */
        int queryCountLimitForSmartMode = -1;

        /**
         * @brief Exact moment when query execution started.
         *
         * Expressed in number of milliseconds since 1970-01-01 00:00:00.
         */
        qint64 simpleExecutionStartTime;

        /**
         * @brief Asynchronous ID of counting query execution.
         *
         * Asynchronous ID returned from Db::asyncExec() for the counting query execution.
         * See countResults() for details on counting query.
         */
        quint32 resultsCountingAsyncId = 0;

        /**
         * @brief Flag indicating results preloading.
         *
         * See Context::preloadResults.
         */
        bool preloadResults = false;

        /**
         * @brief Determinates if asynchronous mode is used.
         *
         * By default QueryExecutor runs in asynchronous mode (in another thread).
         * You can set this to false to make exec() work synchronously, on calling thread.
         */
        bool asyncMode = true;

        /**
         * @brief Defines if the QueryExecutor will provide meta columns in the results.
         *
         * Set to true to forbid providing meta columns, or leave as false to let QueryExecutor
         * provide meta columns.
         *
         * Meta columns are additional columns that are not part of the query that was passed to the executor.
         * Those are for example ROWID columns (currently those are the only meta columns).
         *
         * You can always find out number of ROWID columns from getRowIdResultColumns().
         *
         * Meta columns are placed always at the begining.
         */
        bool noMetaColumns = false;

        /**
         * @brief List of required databases to attach.
         *
         * List of database names (symbolic names, as they appear on the list) that needs to be
         * attached to access all columns returned by the most recent successful execution.
         *
         * This is set after every successful execution.
         */
        QStringList requiredDbAttaches;

        /**
         * @brief Chain of executor steps.
         *
         * Executor step list is set up by setupExecutionChain() and cleaned up after
         * execution is finished. Steps are executed in order they appear in this list.
         *
         * Steps are executed one by one and if any of them raises the error,
         * execution stops and error from QueryExecutor is raised (with executionFailed() signal).
         */
        QList<QueryExecutorStep*> executionChain;

        /**
         * @brief List of registered additional steps to be performed
         *
         * It's a map, where keys describe where in chain should the steps be placed
         * and values are list of steps to be inserted at that position.
         *
         * They are added/removed by methods: registerStep() and deregisterStep().
         */
        static QHash<StepPosition, QList<QueryExecutorStep*>> additionalStatelessSteps;

        /**
         * @brief List of all registered additional steps
         *
         * This is a list of all registered additional steps (registered instances of steps), regardless of their position.
         * This is used to identify registered stateless steps, so they are not deleted on query executor cleanup.
         *
         * Only steps created with a factory are deleted upon executor cleanup (and of course all internal steps created by the executor itself).
         */
        static QList<QueryExecutorStep*> allAdditionalStatelsssSteps;

        /**
         * @brief List of registered factories that create additional steps to be performed
         *
         * This is similar to additionalSteps, except it holds factories, instead of step instances.
         *
         * Steps created with a factory are appended after stateless steps registered as direct
         * instances (held by additionalSteps) - that is for case when both instance and factory
         * are registered for the same step position.
         */
        static QHash<StepPosition, QList<StepFactory*>> additionalStatefulStepFactories;

        /**
         * @brief Execution results handler.
         *
         * This member keeps address of function that was defined for handling results.
         * It is defined only if exec() method was called with the handler function argument.
         *
         * Results handler function is evaluated once the query execution has finished
         * with success. It's not called on failure.
         */
        Db::QueryResultsHandler resultsHandler = nullptr;

        /**
         * @brief Parameters for query execution.
         *
         * It's defined by setParam().
         */
        QHash<QString,QVariant> queryParameters;

        bool forceSimpleMode = false;
        ChainExecutor* simpleExecutor = nullptr;

    signals:
        /**
         * @brief Emitted on successful query execution.
         * @param results Results from query execution.
         *
         * It's emitted at the very end of the whole query execution process
         * and only on successful execution. It doesn't matter if the execution was
         * performed in "smart mode" or "simple mode".
         */
        void executionFinished(SqlQueryPtr results);

        /**
         * @brief Emitted on failed query execution.
         * @param code Error code.
         * @param errorMessage Error message.
         *
         * It doesn't matter if the execution was performed in "smart mode" or "simple mode".
         */
        void executionFailed(int code, QString errorMessage);

        /**
         * @brief Emitted on successful counting query execution.
         * @param rowsAffected Rows affected by the original query.
         * @param rowsReturned Rows returned by the original query.
         * @param totalPages Number of pages needed to represent all rows given the value defined with setResultsPerPage().
         *
         * This signal is emitted only when setSkipRowCounting() was set to false (it is by default)
         * and the counting query execution was successful.
         *
         * The counting query actually counts only \p rowsReturned, while \p rowsAffected and \p totalPages
         * are extracted from original query execution.
         */
        void resultsCountingFinished(quint64 rowsAffected, quint64 rowsReturned, int totalPages);

    public slots:
        /**
         * @brief Executes given query.
         * @param originalQuery to be executed.
         *
         * This is a shorthand for:
         * @code
         * queryExecutor->setQuery(query);
         * queryExecutor->exec();
         * @endcode
         *
         * This exec() version is a SLOT, while the other exec() method is not.
         */
        void exec(const QString& originalQuery);

    private slots:
        /**
         * @brief Calledn when an executor step has failed with its job.
         *
         * An executor step reported an error. "Smart execution" failed and now the executor will try
         * to execute query with a "simple method".
         */
        void stepFailed(QueryExecutorStep *currentStep);

        /**
         * @brief Cleanup routines after failed query execution.
         * @param code Error code.
         * @param errorMessage Error message.
         *
         * Releases resources that are no longer used. Currently simply calls cleanup().
         */
        void cleanupAfterExecFailed(int code, QString errorMessage);

        /**
         * @brief Called when the currently set db is about to be destroyed.
         *
         * Deletes results from the Context if there were any, so they are not referencing anything
         * from deleted Db. Keeping results is dangerous, becuase the Db driver (plugin) is most likely to
         * be unloaded soon and we won't be able to call results destructor.
         */
        void cleanupBeforeDbDestroy(Db* dbToBeUnloaded);

        /**
         * @brief Handles results of simple execution.
         * @param results Results object returned from Db.
         *
         * Checks results for errors and extracts basic meta information,
         * such as rows affected, total result rows and time of execution.
         *
         * In case of success emits executionFinished(), in case of error emits executionFailed().
         */
        void simpleExecutionFinished(SqlQueryPtr results);

        /**
         * @brief Handles asynchronous database execution results.
         * @param asyncId Asynchronous ID of the execution.
         * @param results Results from the execution.
         *
         * QueryExecutor checks whether the \p asyncId belongs to the counting query execution,
         * or the simple execution.
         * Dispatches query results to a proper handler method.
         */
        void dbAsyncExecFinished(quint32 asyncId, SqlQueryPtr results);
};

int qHash(QueryExecutor::EditionForbiddenReason reason);
int qHash(QueryExecutor::ColumnEditionForbiddenReason reason);
int qHash(QueryExecutor::SourceTable sourceTable);
int operator==(const QueryExecutor::SourceTable& t1, const QueryExecutor::SourceTable& t2);

#endif // QUERYEXECUTOR_H
