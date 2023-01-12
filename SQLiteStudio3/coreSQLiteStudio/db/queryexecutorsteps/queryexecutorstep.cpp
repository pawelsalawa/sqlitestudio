#include "queryexecutorstep.h"
#include "db/queryexecutor.h"

QueryExecutorStep::~QueryExecutorStep()
{
}

void QueryExecutorStep::init(QueryExecutor *queryExecutor, QueryExecutor::Context *context)
{
    this->queryExecutor = queryExecutor;
    this->context = context;
    db = queryExecutor->getDb();
    init();
}

void QueryExecutorStep::updateQueries()
{
    QString newQuery;
    for (SqliteQueryPtr& query : context->parsedQueries)
    {
        newQuery += query->detokenize();
        newQuery += "\n";
    }
    context->processedQuery = newQuery;
}

QString QueryExecutorStep::getNextColName()
{
    return "ResCol_" + QString::number(context->colNameSeq++);
}

SqliteSelectPtr QueryExecutorStep::getSelect()
{
    SqliteQueryPtr lastQuery = context->parsedQueries.last();
    if (lastQuery->queryType != SqliteQueryType::Select)
        return SqliteSelectPtr();

    return lastQuery.dynamicCast<SqliteSelect>();
}

void QueryExecutorStep::init()
{
}

TokenList QueryExecutorStep::wrapSelect(const TokenList& selectTokens, const TokenList& resultColumnsTokens)
{
    TokenList oldSelectTokens = selectTokens;
    oldSelectTokens.trimRight(Token::OPERATOR, ";");

    TokenList newTokens;
    newTokens << TokenPtr::create(Token::KEYWORD, "SELECT")
              << TokenPtr::create(Token::SPACE, " ");
    newTokens += resultColumnsTokens;
    newTokens << TokenPtr::create(Token::SPACE, " ")
              << TokenPtr::create(Token::KEYWORD, "FROM")
              << TokenPtr::create(Token::SPACE, " ")
              << TokenPtr::create(Token::PAR_LEFT, "(");
    newTokens += oldSelectTokens;
    newTokens << TokenPtr::create(Token::PAR_RIGHT, ")");
    return newTokens;
}
