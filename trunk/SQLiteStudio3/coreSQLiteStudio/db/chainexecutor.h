#ifndef CHAINEXECUTOR_H
#define CHAINEXECUTOR_H

#include "db/db.h"
#include <QObject>

// TODO add parameters support for ChainExecutor.
// it requires clever api, cause there can be multiple queries and each can use differend parameters,
// while we cannot apply same parameter set for each query, cause they are bind by an available list/hash.

/**
 * @brief Simple query executor, which executes queries one by one.
 *
 * A query executor which lets you execute query (or many queries)
 * using asynchronous execution and it gets back to the called only when
 * all queries succeeded, or it failed at some query.
 *
 * This is very useful if there is a sequence of queries to be executed
 * and you're interested only in the result of the last query.
 *
 * It also lets to configure if queries should be executed within transaction,
 * or not.
 */
class API_EXPORT ChainExecutor : public QObject
{
        Q_OBJECT

    public:
        /**
         * @brief Creates executor.
         * @param parent Parent object for QObject.
         */
        explicit ChainExecutor(QObject *parent = 0);

        /**
         * @brief Tells if transactional execution is enabled.
         * @return Transactional execution status.
         */
        bool getTransaction() const;

        /**
         * @brief Enabled or disables transactional execution.
         * @param value True to enable, false to disable.
         *
         * Transactional execution is enabled by default. It means that all defined SQL queries
         * will be executed in single SQL transaction.
         */
        void setTransaction(bool value);

        /**
         * @brief Provides list of SQL queries configured for this executor.
         * @return List of queries.
         */
        QStringList getQueries() const;

        /**
         * @brief Defines list of queries to be executed.
         * @param value List of query strings.
         *
         * This is the main mathod you're interested in when using ChainExecutor.
         * This is how you define what SQL queries will be executed.
         *
         * Calling this method will clear any parameters defined previously with setParam().
         */
        void setQueries(const QStringList& value);

        /**
         * @brief Provides currently configured database.
         * @return Database that the queries are executed on in this executor.
         */
        Db* getDb() const;

        /**
         * @brief Defines database for executing queries.
         * @param value The database object.
         *
         * It is necessary to define the database before executing queries,
         * otherwise the start() will emit failure() signal and do nothing else.
         */
        void setDb(Db* value);

        /**
         * @brief Provides list of configured query mandatory flags.
         * @return List of flags.
         *
         * See setMandatoryQueries() for details on mandatory flags.
         */
        QList<bool> getMandatoryQueries() const;

        /**
         * @brief Defines list of mandatory flags for queries.
         * @param value List of flags - a boolean per each defined query.
         *
         * Setting mandatory flags lets you define which queries (defined with setSqls())
         * are mandatory for the successful execution and which are not.
         * Queries are mandatory by default (when flags are not defined),
         * which means that every defined query execution must be successfull,
         * otherwise executor breaks the execution chain and reports error.
         *
         * By defining mandatory flags to false for some queries, you're telling
         * to ChainExecutor, that it's okay if those queries fail and it should
         * move along.
         *
         * For example:
         * @code
         * ChainExecutor executor;
         * executor.setSqls({
         *     "DELETE FROM table1 WHERE value = 5",
         *     "DELETE FROM possibly_not_existing_table WHERE column > 3",
         *     "INSERT INTO table1 VALUES (4, 6)"
         * });
         * executor.setMandatoryQueries({true, false, true});
         * @endcode
         * We defined second query to be optional, therefore if the table
         * "possibly_not_existing_table" doesn't exist, that's fine.
         * It will be ignored and the third query will be executed.
         * If flags were not defined, then execution of second query would fail,
         * executor would stop there, report error (with failure() signal)
         * and the third query would not be executed.
         *
         * It also affects transactions. If executor was defined to execute
         * in a transaction (with setTransaction()), then failed query
         * that was not mandatory will also not rollback the transaction.
         *
         * In other words, queries marked as not mandatory are silently ignored
         * when failed.
         */
        void setMandatoryQueries(const QList<bool>& value);

        /**
         * @brief Provides list of execution error messages.
         * @return List of messages.
         *
         * Execution error messages usually have zero or one message,
         * but if you defined some queries to be not mandatory,
         * then each failed optional query will be silently ignored,
         * but its error message will be stored and returned by this method.
         * In that case, the result of this method can provide more than
         * one message.
         */
        QStringList getExecutionErrors() const;

        /**
         * @brief Tells if the executor is configured for asynchronous execution.
         * @return Asynchronous flag value.
         */
        bool getAsync() const;

        /**
         * @brief Defines asynchronous execution mode.
         * @param value true to enable asynchronous execution, false to disable it.
         *
         * Asynchronous execution causes start() to return immediately.
         *
         * When asynchronous mode is enabled, results of execution
         * have to be handled by connecting to failed() and success() signals.
         *
         * If the asynchronous mode is disabled, result can be queried
         * by getSuccessfulExecution() call.
         */
        void setAsync(bool value);

        /**
         * @brief Tells if the most recent execution was successful.
         * @return true if execution was successful, or false if it failed.
         *
         * Successful execution means that all mandatory queries
         * (see setMandatoryQueries()) executed successfully.
         * Optional (not mandatory) queries do not affect result of this method.
         *
         * If this method returns true, it also means that success() signal
         * was emitted.
         * If this method returns false, it also means that failure() signal
         * was emitted.
         */
        bool getSuccessfulExecution() const;

        /**
         * @brief Defines named parameter to bind in queries.
         * @param paramName Parameter name (must include the preceding ':').
         * @param value Value for the parameter.
         *
         * Any parameter defined with this method will be applied to each query
         * executed by the executor. If some query doesn't include parameter
         * placeholder with defined name, then the parameter will simply
         * not be applied to that query.
         */
        void setParam(const QString& paramName, const QVariant& value);

    private:
        /**
         * @brief Executes query defines as the current one.
         *
         * Checks is there is a current query defined (pointed by currentSqlIndex).
         * If there is, then executes it. If not, goes to executionSuccessful().
         *
         * This is called for each next query in asynchronous mode.
         */
        void executeCurrentSql();

        /**
         * @brief Handles failed execution.
         * @param errorCode Error code.
         * @param errorText Error message.
         *
         * Rolls back transaction (in case of transactional execution) and emits failure().
         */
        void executionFailure(int errorCode, const QString& errorText);

        /**
         * @brief Handles successful execution.
         *
         * Commits transaction (in case of transactional execution) and emits success().
         */
        void executionSuccessful();

        /**
         * @brief Executes all queries synchronously.
         *
         * If the asynchronous mode is disabled, then this method executes all queries.
         */
        void executeSync();

        /**
         * @brief Handles single query execution results.
         * @param results Results from the query.
         * @return true if the execution was successful, or false otherwise.
         *
         * If there was an error while execution, then executionFailure() is also called.
         */
        bool handleResults(SqlResultsPtr results);

        /**
         * @brief Database for execution.
         */
        Db* db = nullptr;

        /**
         * @brief Transactional execution mode.
         */
        bool transaction = true;

        /**
         * @brief Asynchronous execution mode.
         */
        bool async = true;

        /**
         * @brief Queries to be executed.
         */
        QStringList sqls;

        /**
         * @brief List of flags for mandatory queries.
         *
         * See setMandatoryQueries() for details.
         */
        QList<bool> mandatoryQueries;

        /**
         * @brief Index pointing to the current query in sqls list.
         *
         * When executing query in asynchronous mode, this index points to the next
         * query that should be executed.
         */
        int currentSqlIndex = -1;

        /**
         * @brief Asynchronous ID of current query execution.
         *
         * The ID is provided by Db::asyncExec().
         */
        quint32 asyncId = -1;

        /**
         * @brief Execution interrupted flag.
         *
         * Once the interrup() was called, this flag is set to true,
         * so the executor knows that it should not execute any further queries.
         */
        bool interrupted = false;

        /**
         * @brief Errors raised during queries execution.
         *
         * In case of major failure, the error message is appended to this list,
         * but when mandatory flags allow some failures, than this list may
         * contain more error messages.
         */
        QStringList executionErrors;

        /**
         * @brief Successful execution indicator.
         *
         * This is set after execution is finished.
         */
        bool successfulExecution = false;

        /**
         * @brief Parameters to bind to queries.
         *
         * This is filled with setParam() calls and used later to bind
         * parameters to executed queries.
         */
        QHash<QString,QVariant> queryParams;

    public slots:
        /**
         * @brief Interrupts query execution.
         */
        void interrupt();

        /**
         * @brief Starts execution of all defined queries, one by one.
         */
        void exec();

    private slots:
        /**
         * @brief Handles asynchronous execution results from Db::asyncExec().
         * @param asyncId Asynchronous ID of the execution for the results.
         * @param results Results returned from execution.
         *
         * Checks if given asynchronous ID matches the internally stored asyncId
         * and if yes, then handles results and executes next query in the queue.
         */
        void handleAsyncResults(quint32 asyncId, SqlResultsPtr results);

    signals:
        /**
         * @brief Emitted when all mandatory queries were successfully executed.
         *
         * See setMandatoryQueries() for details on mandatory queries.
         */
        void success();

        /**
         * @brief Emitted when major error occurred while executing a query.
         * @param errorCode Error code.
         * @param errorText Error message.
         *
         * It's emitted only when mandatory query has failed execution.
         * See setMandatoryQueries() for details on mandatory queries.
         */
        void failure(int errorCode, const QString& errorText);
};

#endif // CHAINEXECUTOR_H
