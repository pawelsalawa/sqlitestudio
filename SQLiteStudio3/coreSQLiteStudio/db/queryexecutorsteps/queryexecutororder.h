#ifndef QUERYEXECUTORORDER_H
#define QUERYEXECUTORORDER_H

#include "queryexecutorstep.h"

/**
 * @brief Applies sorting to the query.
 *
 * Sorting is done by enclosing SELECT query with another SELECT
 * query, but the outer one uses order defined in this step.
 *
 * The order is defined by QueryExecutor::setSortOrder().
 */
class QueryExecutorOrder : public QueryExecutorStep
{
        Q_OBJECT

    public:
        bool exec();

    private:
        /**
         * @brief Generates tokens to sort by given column and order.
         * @param sortOrder Definition of order to use.
         * @return Tokens to append to the SELECT.
         */
        TokenList getOrderTokens(const QueryExecutor::SortList& sortOrder);
};

#endif // QUERYEXECUTORORDER_H
