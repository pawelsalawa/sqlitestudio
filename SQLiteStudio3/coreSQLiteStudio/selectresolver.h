#ifndef SELECTRESOLVER_H
#define SELECTRESOLVER_H

#include "common/strhash.h"
#include "parser/ast/sqliteselect.h"
#include "common/bistrhash.h"
#include "parser/ast/sqlitewith.h"
#include <QString>
#include <QHash>
#include <QStringList>
#include <QStack>

class Db;
class SchemaResolver;

/**
 * @brief Result column introspection tool
 * The SelectResolver provides full information about what would
 * result columns be for given SELECT statement. It investigates
 * deeply "FROM" clause and the typed result column list
 * and in produces list of column objects where each of them
 * is described by it's source table, column name in that table,
 * a database and a column alias (if any).
 *
 * The database is a name database as seen by SQLite, which is
 * for example "main", "temp", or any name used with "ATTACH".
 *
 * If the column is not related to the table, but instead is
 * an expression, then it's type is set to "OTHER".
 *
 * If column's table is named with an alias, then it is also provided.
 *
 * The displayName field describes how would the column be named
 * by SQLite itself if it was returned in query results.
 *
 * The returned column object has also a reference to original
 * SqliteSelect::Core::ResultColumn object, so that one can relate
 * which queried column produced given column object in this class.
 *
 * Result column like "table.*" will produce one or more column
 * objects from this class.
 *
 * In case of CTE (Common Table Expression, aka WITH statement)
 * the columns resolved from CTE will by of type COLUMN
 * and will have FROM_CTE flag set. Such columns will have
 * displayName and tableAlias populated, while database, table
 * and column attributes may or may not be populated.
 *
 * There's one unsupported case: When the select has a subselect
 * in "FROM" clause and that subselect is actually a multi-core
 * select (with UNIONs), then columns produced from such source
 * won't be related to any table, because currently it's impossible
 * for SelectResolver to tell from which table of multi-core
 * subselect the column is read from. Therefore in this case
 * the column object has it's name, but no table or database.
 */
class API_EXPORT SelectResolver
{
    public:
        enum Flag
        {
            FROM_COMPOUND_SELECT = 0x01,
            FROM_ANONYMOUS_SELECT = 0x02,
            FROM_DISTINCT_SELECT = 0x04,
            FROM_GROUPED_SELECT = 0x08,
            FROM_CTE_SELECT = 0x10,
            FROM_VIEW = 0x20,
            FROM_TABLE_VALUED_FN = 0x40,
            FROM_RES_COL_SUBSELECT = 0x80, // for result columns that are in-line subselects (i.e. subselect that's not in FROM, but in result columns)
        };

        /**
         * @brief Table resolved by the resolver.
         */
        struct API_EXPORT Table
        {
            /**
             * @brief Database name.
             *
             * Either sqlite name, like "main", or "temp", or an attach name.
             */
            QString database;
            QString originalDatabase;
            QString table;
            QString tableAlias;
            QStringList oldTableAliases;
            int flags = 0;

            int operator==(const Table& other);
            void pushTableAlias();
        };

        /**
         * @brief Result column resolved by the resolver.
         */
        struct API_EXPORT Column : public Table
        {
            enum Type
            {
                COLUMN,
                OTHER
            };

            Type type;

            /**
             * @brief Column name or expression.
             *
             * If a column is of OTHER type, then column member contains detokenized column expression.
             */
            QString column;
            QString alias;
            QString displayName;
            bool aliasDefinedInSubQuery = false;

            int operator==(const Column& other);
            Table getTable() const;
        };

        SelectResolver(Db* db, const QString &originalQuery);
        SelectResolver(Db* db, const QString &originalQuery, const BiStrHash& dbNameToAttach);
        ~SelectResolver();

        QList<Column> resolveColumnsFromFirstCore();
        QList<QList<Column>> resolveColumns();

        QList<Column> resolve(SqliteSelect::Core* selectCore);
        QList<QList<Column>> resolve(SqliteSelect* select);

        QList<Column> resolveAvailableColumns(SqliteSelect::Core* selectCore);
        QList<QList<Column>> resolveAvailableColumns(SqliteSelect* select);
        QList<Column> resolveAvailableColumns(SqliteSelect::Core::JoinSource* joinSrc);

        QSet<Table> resolveTables(SqliteSelect::Core* selectCore);
        QList<QSet<Table>> resolveTables(SqliteSelect* select);
        QSet<Table> resolveTables(SqliteSelect::Core::JoinSource* joinSrc);

        /**
         * @brief Translates tokens representing column name in the SELECT into full column objects.
         * @param select Select statement containing all queried tokens.
         * @param columnTokens Column tokens to translate.
         * @return Full column objects with table and database fields filled in, unless translation failed for some column.
         * This method lets you to get full information about columns being specified anywhere in the select,
         * no matter if they were typed with database and table prefix or not. It uses smart algorighms
         * from the very same class to resolve all available columns, given all data sources of the select,
         * finds the queried column token in those available columns  and creates full column object.
         * Every column token passed to the method will result in a column objects in the results.
         * If for some reason the translation was not possible, then the respective column in the results will
         * have an OTHER type, while successfully translated columns will have a COLUMN type.
         *
         * In other words, given the column names (as tokens), this methods finds an occurance of this token in
         * the given select statement and provides information in what context was the column used (database, table).
         *
         * This method is used for example in TableModifier to find out what columns from the modified table
         * were used in the referencing CREATE VIEW statements.
         */
        QList<Column> translateToColumns(SqliteSelect* select, const TokenList& columnTokens);

        /**
         * @brief Translates token representing column name in the SELECT into full column objects.
         * @param select Select statement containing queried token.
         * @param token Column token to translate.
         * @return Full column object with table and database fields filled in.
         * This method does pretty much the same thing as #translateToColumns(SqliteSelect*,const TokenList&),
         * except it takes only one token as an argument.
         *
         * Internally this method is used by #translateToColumns(SqliteSelect*,const TokenList&) in a loop.
         */
        Column translateToColumns(SqliteSelect* select, TokenPtr token);

        /**
         * @brief Tells whether there were any errors during resolving.
         * @return true if there were any errors, or false otherwise.
         */
        bool hasErrors() const;

        /**
         * @brief Provides list of errors encountered during resolving.
         * @return List of localized error messages.
         */
        const QStringList& getErrors() const;

        /**
         * @brief This is a convenient overload of method that uses database object passed to the constructor.
         * @param query SQL query to analyze.
         * @return Column with limited metadata (database, table name - if applicable, column name - if applicable, and displayName)
         *
         * See static overloaded method for more details.
         */
        QList<Column> sqliteResolveColumns(const QString& query);

        /**
         * @brief Does quick analysis of result columns for the query, using SQLite built-in API.
         * @param db Database to analyze the query against.
         * @param query SQL query to analyze.
         * @param dbNameToAttach A mapping of database name to its attach name, used to resolve db name if SQLite API returns it as attach name.
         * @return Column with limited metadata (database, table name - if applicable, column name - if applicable, and displayName)
         *
         * Instead of doing full scale anaysis of the query, it leverages SQLite API for a query metadata,
         * but it lacks information about source table aliases, subquery aliases,
         * flags (compund query, aggregate query, cte query, etc).
         * It's convenient, but must be complemented by sophisticated logic of the SchemaResolver to provide full info.
         * Yet it's good enough if these extended information are not needed, as it can be much faster, than full analysis.
         */
        static QList<Column> sqliteResolveColumns(Db* db, const QString& query, const BiStrHash& dbNameToAttach = BiStrHash());

        /**
         * @brief resolveMultiCore
         * If true (by default), the multi-core subselects will be resolved using
         * first core from the list. If false, then the subselect will be ignored by resolver,
         * which will result in empty columns and/or tables resolved for that subselect.
         */
        bool resolveMultiCore = true;

        /**
         * @brief ignoreInvalidNames
         * If true, then problems with resolving real objects in sqlite_master mentioned in the select,
         * are ignored silently. Otherwise those accidents are reported with qDebug().
         */
        bool ignoreInvalidNames = false;

    private:
        QList<Column> resolveCore(SqliteSelect::Core* selectCore);
        QList<Column> resolveAvailableCoreColumns(SqliteSelect::Core* selectCore);
        QSet<Table> resolveTablesFromCore(SqliteSelect::Core* selectCore);
        void markFlagsBySelect(SqliteSelect::Core* core, QList<Column>& columns);
        Column translateTokenToColumn(SqliteSelect* select, TokenPtr token);
        void resolve(SqliteSelect::Core::ResultColumn* resCol);
        void resolveStar(SqliteSelect::Core::ResultColumn* resCol);
        void resolveExpr(SqliteSelect::Core::ResultColumn* resCol);
        void resolveDbAndTable(SqliteSelect::Core::ResultColumn *resCol);
        Column resolveRowIdColumn(SqliteExpr* expr);
        Column resolveExplicitColumn(const QString& columnName);
        Column resolveExplicitColumn(const QString& table, const QString& columnName);
        Column resolveExplicitColumn(const QString& database, const QString& table, const QString& columnName);

        QList<Column> resolveJoinSource(SqliteSelect::Core::JoinSource* joinSrc);
        QList<Column> resolveSingleSource(SqliteSelect::Core::SingleSource* joinSrc);
        QList<Column> resolveCteColumns(SqliteSelect::Core::SingleSource* joinSrc);
        QList<Column> resolveTableFunctionColumns(SqliteSelect::Core::SingleSource* joinSrc);
        QList<Column> resolveSingleSourceSubSelect(SqliteSelect::Core::SingleSource* joinSrc);
        QList<Column> resolveOtherSource(SqliteSelect::Core::JoinSourceOther *otherSrc);
        QList<Column> resolveSubSelect(SqliteSelect* select);
        QList<Column> resolveView(SqliteSelect::Core::SingleSource *joinSrc);
        bool isView(const QString& database, const QString& name);
        QStringList getTableColumns(const QString& database, const QString& table, const QString &alias);
        void applySubSelectAlias(QList<Column>& columns, const QString& alias);
        QString resolveDatabase(const QString& database);
        bool parseOriginalQuery();

        void markDistinctColumns(QList<Column>* columnList = nullptr);
        void markCompoundColumns(QList<Column>* columnList = nullptr);
        void markGroupedColumns(QList<Column>* columnList = nullptr);
        void fixColumnNames();
        void markCurrentColumnsWithFlag(Flag flag, QList<Column>* columnList = nullptr);
        bool matchTable(const Column& sourceColumn, const QString& table);
        TokenList getResColTokensWithoutAlias(SqliteSelect::Core::ResultColumn *resCol);
        void extractCte(SqliteSelect* select);
        void extractCte(SqliteSelect::Core* core);

        Db* db = nullptr;
        QString query;
        SqliteSelectPtr originalQueryParsed;
        StrHash<SqliteWith::CommonTableExpression*> cteList;

        /**
         * @brief Database name to attach name map.
         *
         * When this map is defined, then every occurance of the database in the query will be
         * checked against being an attach name and if it is an attach name, then it will be
         * translated into the original database name used in the query using this map.
         *
         * This will result in original database names in "originalDatabase" and "displayName"
         * members returned from the resolver.
         *
         * The map should be collected when the attaching is performed. For example DbAttacher
         * does the job for you - it attaches databases and prepares the map, that you can
         * use here.
         */
        BiStrHash dbNameToAttach;

        /**
         * @brief currentCoreResults
         * List of columns that will be returned from resultColumns of the queried SELECT.
         * Columns are linked to their tables from "FROM" clause if possible,
         * otherwise they have null table name.
         * This list is built progressively when iterating through result columns.
         * Then it's returned from resolve() call.
         */
        QList<Column> currentCoreResults;

        /**
         * @brief tableColumnsCache
         * When the resolver asks database for list of its columns (by PRAGMA table_info()),
         * then it stores results in this cache, becuase it's very likely that this table
         * will be queried for columns more times.
         */
        QHash<Table,QStringList> tableColumnsCache;

        /**
         * @brief currentCoreSourceColumns
         * List of every column available from current selectCore sources.
         * There can be many columns with same database and table.
         * It can be also interpreted as list of all available tables in the "FROM" clause
         * (but only at the top level, recursive subselects or subjoins).
         * This list is created at the begining of resolve() call, before any result column
         * is being actually resolved. This is also created by resolveTables() call
         * and in that case no result columns are being resolved, just this list is being
         * converted into the list of SqliteStatement::Table and returned.
         *
         * Note, that some entries in this list will have only column name filled in
         * and no table related information - this happens when the column comes from
         * subselect, where it was not a table related result column.
         */
        QList<Column> currentCoreSourceColumns;

        /**
         * @brief schemaResolver
         * Used to get list of column names in a table.
         */
        SchemaResolver* schemaResolver = nullptr;

        /**
         * @brief List of errors encountered during resolving.
         *
         * This may contain errors like missing table alias that was used in result column list, etc.
         */
        QStringList errors;
};

API_EXPORT int operator==(const SelectResolver::Table& t1, const SelectResolver::Table& t2);
API_EXPORT TYPE_OF_QHASH qHash(const SelectResolver::Table& table);

API_EXPORT int operator==(const SelectResolver::Column& c1, const SelectResolver::Column& c2);
API_EXPORT TYPE_OF_QHASH qHash(const SelectResolver::Column& column);

API_EXPORT QDebug operator<<(QDebug debug, const SelectResolver::Column &c);
API_EXPORT QDebug operator<<(QDebug debug, const SelectResolver::Table &c);

#endif // SELECTRESOLVER_H
