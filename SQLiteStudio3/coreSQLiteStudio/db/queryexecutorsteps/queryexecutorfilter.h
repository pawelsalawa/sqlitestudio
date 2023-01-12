#ifndef QUERYEXECUTORFILTER_H
#define QUERYEXECUTORFILTER_H

#include "queryexecutorstep.h"

/**
 * @brief Applies WHERE filtering to the query.
 *
 * This step is executed late in the execution chain. It is useful, when one wants to apply filtering
 * without involving whole column/rowid analysis that is done in earlier executor steps.
 */
class QueryExecutorFilter : public QueryExecutorStep
{
        Q_OBJECT

    public:
        bool exec();
};

#endif // QUERYEXECUTORFILTER_H
