#include "queryexecutorparsequery.h"
#include "db/queryexecutor.h"
#include "parser/parser.h"
#include <QDebug>

QueryExecutorParseQuery::QueryExecutorParseQuery(const QString& name)
    : QueryExecutorStep()
{
    setObjectName(name);
}

QueryExecutorParseQuery::~QueryExecutorParseQuery()
{
    delete parser;
}

bool QueryExecutorParseQuery::exec()
{
    // Prepare parser
    delete parser;

    parser = new Parser();

    // Do parsing
    context->parsedQueries.clear();
    parser->parse(context->processedQuery);
    if (parser->getErrors().size() > 0)
    {
        qWarning() << "QueryExecutorParseQuery:" << parser->getErrorString() << "\n"
                   << "Query parsed:" << context->processedQuery;
        return false;
    }

    if (parser->getQueries().size() == 0)
    {
        qWarning() <<  "No queries parsed in QueryExecutorParseQuery step.";
        return false;
    }

    context->parsedQueries = parser->getQueries();

    // We never want the semicolon in last query, because the query could be wrapped with a SELECT
    context->parsedQueries.last()->tokens.trimRight(Token::OPERATOR, ";");

    return true;
}
