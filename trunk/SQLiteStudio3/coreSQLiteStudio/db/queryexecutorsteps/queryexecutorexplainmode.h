#ifndef QUERYEXECUTOREXPLAINMODE_H
#define QUERYEXECUTOREXPLAINMODE_H

#include "queryexecutorstep.h"

/**
 * @brief Applies QueryExecutor::Context::explainMode to the query.
 *
 * If explain mode is enabled, it just prepends "EXPLAIN" before the query.
 */
class QueryExecutorExplainMode : public QueryExecutorStep
{
        Q_OBJECT
    public:
        bool exec();
};

#endif // QUERYEXECUTOREXPLAINMODE_H
