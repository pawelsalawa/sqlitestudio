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
    if (parser)
        delete parser;
}

bool QueryExecutorParseQuery::exec()
{
    // Prepare parser
    if (parser)
        delete parser;

    parser = new Parser(dialect);

    // Do parsing
    context->parsedQueries.clear();
    parser->parse(context->processedQuery);
    if (parser->getErrors().size() > 0)
    {
        qWarning() << "QueryExecutorParseQuery:" << parser->getErrorString();
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
