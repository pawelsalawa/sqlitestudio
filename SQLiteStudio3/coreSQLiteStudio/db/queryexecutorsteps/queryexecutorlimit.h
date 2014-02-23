#ifndef QUERYEXECUTORLIMIT_H
#define QUERYEXECUTORLIMIT_H

#include "queryexecutorstep.h"

/**
 * @brief Applies row count limit to the query.
 *
 * If row count limit is enabled (QueryExecutor::Context::setPage()
 * and QueryExecutor::Context::setResultsPerPage), then the SELECT query
 * is wrapped with another SELECT which defines it's own LIMIT and OFFSET
 * basing on the page and the results per page parameters.
 */
class QueryExecutorLimit : public QueryExecutorStep
{
        Q_OBJECT

    public:
        bool exec();
};

#endif // QUERYEXECUTORLIMIT_H
