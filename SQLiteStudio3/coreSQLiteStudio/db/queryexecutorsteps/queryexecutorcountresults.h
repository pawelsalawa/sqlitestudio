#ifndef QUERYEXECUTORCOUNTRESULTS_H
#define QUERYEXECUTORCOUNTRESULTS_H

#include "queryexecutorstep.h"

/**
 * @brief Defines counting query string.
 *
 * @see QueryExecutor::countResults()
 */
class QueryExecutorCountResults : public QueryExecutorStep
{
        Q_OBJECT

    public:
        bool exec();
};

#endif // QUERYEXECUTORCOUNTRESULTS_H
