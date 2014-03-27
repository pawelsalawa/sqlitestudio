#include "queryexecutorexecute.h"
#include "db/sqlerrorcodes.h"
#include "db/queryexecutor.h"
#include "parser/ast/sqlitequery.h"
#include "parser/lexer.h"
#include <QDateTime>
#include <QDebug>

bool QueryExecutorExecute::exec()
{
//    qDebug() << "q:" << context->processedQuery;

    startTime = QDateTime::currentMSecsSinceEpoch();
    return executeQueries();
}

void QueryExecutorExecute::provideResultColumns(SqlResultsPtr results)
{
    QueryExecutor::ResultColumnPtr resCol = QueryExecutor::ResultColumnPtr::create();
    foreach (const QString& colName, results->getColumnNames())
    {
        resCol->displayName = colName;
        context->resultColumns << resCol;
    }
}

bool QueryExecutorExecute::executeQueries()
{
    QHash<QString, QVariant> bindParamsForQuery;
    SqlResultsPtr results;

    Db::Flags flags;
    if (context->preloadResults)
        flags |= Db::Flag::PRELOAD;

    foreach (const SqliteQueryPtr& query, context->parsedQueries)
    {
        bindParamsForQuery = getBindParamsForQuery(query);
        results = db->exec(query->detokenize(), bindParamsForQuery, flags);

        if (results->isError())
        {
            handleFailResult(results);
            return false;
        }
    }
    handleSuccessfulResult(results);
    return true;
}

void QueryExecutorExecute::handleSuccessfulResult(SqlResultsPtr results)
{
    SqliteSelectPtr select = getSelect();
    if (!select || select->coreSelects.size() > 1 || select->explain)
    {
        // In this case, the "Columns" step didn't provide result columns.
        // We need to do it here, basing on actual results.
        provideResultColumns(results);
    }

    context->executionTime = QDateTime::currentMSecsSinceEpoch() - startTime;
    context->rowsAffected = results->rowsAffected();

    // For PRAGMA and EXPLAIN we simply count results for rows returned
    SqliteQueryPtr lastQuery = context->parsedQueries.last();
    if (lastQuery->queryType == SqliteQueryType::Pragma || lastQuery->explain)
        context->rowsCountingRequired = true;

    if (context->resultsHandler)
    {
        context->resultsHandler(results);
        context->resultsHandler = nullptr;
    }

    context->executionResults = results;
}

void QueryExecutorExecute::handleFailResult(SqlResultsPtr results)
{
    if (!results->isInterrupted())
    {
        qWarning() << "Could not execute query with smart method:" << queryExecutor->getOriginalQuery()
                   << "\nError message:" << results->getErrorText()
                   << "\nSkipping smart execution.";
    }
}

QHash<QString, QVariant> QueryExecutorExecute::getBindParamsForQuery(SqliteQueryPtr query)
{
    QHash<QString, QVariant> queryParams;
    QStringList bindParams = query->tokens.filter(Token::BIND_PARAM).toStringList();
    foreach (const QString& bindParam, bindParams)
    {
        if (context->queryParameters.contains(bindParam))
            queryParams.insert(bindParam, context->queryParameters[bindParam]);
    }
    return queryParams;
}
