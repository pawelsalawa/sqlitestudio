#ifndef QUERYEXECUTORPARSEQUERY_H
#define QUERYEXECUTORPARSEQUERY_H

#include "queryexecutorstep.h"

/**
 * @brief Parses current query and stores results in the context.
 *
 * Parses QueryExecutor::Context::processedQuery and stores results
 * in QueryExecutor::Context::parsedQueries.
 *
 * This is used after some changes were made to the query and next steps will
 * require parsed representation of queries to be updated.
 */
class QueryExecutorParseQuery : public QueryExecutorStep
{
        Q_OBJECT

    public:
        explicit QueryExecutorParseQuery(const QString& name);
        ~QueryExecutorParseQuery();

        bool exec();

    private:
        Parser* parser = nullptr;
};

#endif // QUERYEXECUTORPARSEQUERY_H
