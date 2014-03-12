#include "queryexecutorstep.h"
#include "db/queryexecutor.h"
#include "common/unused.h"

QueryExecutorStep::~QueryExecutorStep()
{
}

void QueryExecutorStep::init(QueryExecutor *queryExecutor, QueryExecutor::Context *context)
{
    this->queryExecutor = queryExecutor;
    this->context = context;
    db = queryExecutor->getDb();
    dialect = db->getDialect();
    init();
}

void QueryExecutorStep::updateQueries()
{
    QString newQuery;
    foreach (SqliteQueryPtr query, context->parsedQueries)
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

