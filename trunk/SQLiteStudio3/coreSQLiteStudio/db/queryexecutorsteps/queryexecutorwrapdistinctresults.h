#ifndef QUERYEXECUTORWRAPDISTINCTRESULTS_H
#define QUERYEXECUTORWRAPDISTINCTRESULTS_H

#include "queryexecutorstep.h"

/**
 * @brief The QueryExecutorWrapDistinctResults class
 *
 * This step is necessary for DISTINCT and GROUP BY selects,
 * because result columns are always limited by substr() function,
 * which makes INT and TEXT types of data to be the same, which is false.
 * For those cases, we need to put DISTINCT/GROUP BY into subselect,
 * so it works on original result columns, then get the substr() from them.
 *
 * Processed query is stored in QueryExecutor::Context::processedQuery.
 */
class QueryExecutorWrapDistinctResults : public QueryExecutorStep
{
        Q_OBJECT
    public:
        bool exec();

    private:
        /**
         * @brief Wraps single SELECT statement.
         * @param select SELECT to wrap.
         *
         * Puts given \p select as subselect of a new SELECT.
         */
        void wrapSelect(SqliteSelect* select);
};

#endif // QUERYEXECUTORWRAPDISTINCTRESULTS_H
