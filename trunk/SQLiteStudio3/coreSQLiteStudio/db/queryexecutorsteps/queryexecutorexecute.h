#ifndef QUERYEXECUTOREXECUTE_H
#define QUERYEXECUTOREXECUTE_H

#include "queryexecutorstep.h"
#include <QHash>

/**
 * @brief Executes query in current form.
 *
 * Executes query synchronously (since entire query executor works in another thread anyway).
 * After execution is finished it provides information about how long it took, whether there was
 * an error, and how many rows were affected/returned.
 *
 * The query string may contain many queries separated by semicolon and this step will split
 * them correctly, then execute one-by-one. Results are loaded only from last query execution.
 *
 * If the last query was not processed by QueryExecutorColumns step, then this step
 * will provide list result column names basing on what names returned SQLite.
 *
 * For PRAGMA and EXPLAIN statements rows returned are not accurate
 * and QueryExecutor::Context::rowsCountingRequired is set to true.
 */
class QueryExecutorExecute : public QueryExecutorStep
{
        Q_OBJECT

    public:
        bool exec();

    private:
        /**
         * @brief Gives list of column names as SQLite returned them.
         * @param results Execution results.
         */
        void provideResultColumns(SqlResultsPtr results);

        /**
         * @brief Executes the query.
         * @return true on success, false on failure.
         *
         * Stops on first error and in that case rolls back transaction.
         *
         * If QueryExecutor::Context::preloadResults is true, then also Db::Flag::PRELOAD
         * is appended to execution flags.
         */
        bool executeQueries();

        /**
         * @brief Extracts meta information from results.
         * @param results Execution results.
         *
         * Meta information includes rows affected, execution time, etc.
         */
        void handleSuccessfulResult(SqlResultsPtr results);

        /**
         * @brief Handles failed execution.
         * @param results Execution results.
         *
         * Currently this method doesn't do much. It just checks whether execution
         * error was caused by call to Db::interrupt(), or not and if not,
         * then the warning is logged about it and executor falls back to simple
         * execution method.
         */
        void handleFailResult(SqlResultsPtr results);

        /**
         * @brief Prepares parameters for query execution.
         * @param query Query to be executed.
         * @return Map of parameters for the query.
         *
         * It generates parameters basing on what are parameter placeholders in the query
         * and what are parameter values available in QueryExecutor::Context::queryParameters.
         */
        QHash<QString, QVariant> getBindParamsForQuery(SqliteQueryPtr query);

        /**
         * @brief Number of milliseconds since 1970 at execution start moment.
         */
        qint64 startTime;
};

#endif // QUERYEXECUTOREXECUTE_H
