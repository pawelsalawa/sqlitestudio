#include "queryexecutorexecute.h"
#include "db/sqlerrorcodes.h"
#include "db/queryexecutor.h"
#include "parser/ast/sqlitequery.h"
#include "parser/lexer.h"
#include "log.h"
#include "parser/ast/sqlitecreatetable.h"
#include "datatype.h"
#include "schemaresolver.h"
#include "common/table.h"
#include <QDateTime>
#include <QDebug>
#include <QStack>

bool QueryExecutorExecute::exec()
{
    //qDebug() << "q:" << context->processedQuery;

    startTime = QDateTime::currentMSecsSinceEpoch();
    return executeQueries();
}

void QueryExecutorExecute::provideResultColumns(SqlQueryPtr results)
{
    QueryExecutor::ResultColumnPtr resCol;
    for (const QString& colName : results->getColumnNames())
    {
        resCol = QueryExecutor::ResultColumnPtr::create();
        resCol->displayName = colName;
        context->resultColumns << resCol;
    }
}

bool QueryExecutorExecute::executeQueries()
{
    QHash<QString, QVariant> bindParamsForQuery;
    SqlQueryPtr results;
    context->rowsAffected = 0;
    QStack<int> rowsAffectedBeforeTransaction;

    Db::Flags flags;
    if (context->preloadResults)
        flags |= Db::Flag::PRELOAD;

    QString queryStr;
    for (const SqliteQueryPtr& query : context->parsedQueries)
    {
        queryStr = query->detokenize();
        bindParamsForQuery = getBindParamsForQuery(query);
        results = db->prepare(queryStr);
        results->setArgs(bindParamsForQuery);
        results->setFlags(flags);

        if (isBeginTransaction(query->queryType))
            rowsAffectedBeforeTransaction.push(context->rowsAffected);

        //qDebug() << getLogDateTime() << "Executing query:" << queryStr;
        results->execute();
        //qDebug() << getLogDateTime() << "Done.";

        if (results->isError())
        {
            handleFailResult(results);
            return false;
        }

        context->rowsAffected += results->rowsAffected();

        if (rowsAffectedBeforeTransaction.size() > 0)
        {
            if (isCommitTransaction(query->queryType))
                rowsAffectedBeforeTransaction.pop();
            else if (isRollbackTransaction(query->queryType))
                context->rowsAffected = rowsAffectedBeforeTransaction.pop();
        }
    }
    handleSuccessfulResult(results);
    return true;
}

void QueryExecutorExecute::handleSuccessfulResult(SqlQueryPtr results)
{
    SqliteSelectPtr select = getSelect();
    if (!select || select->coreSelects.size() > 1 || select->explain)
    {
        // In this case, the "Columns" step didn't provide result columns.
        // We need to do it here, basing on actual results.
        provideResultColumns(results);
    }

    context->executionTime = QDateTime::currentMSecsSinceEpoch() - startTime;

    // For PRAGMA and EXPLAIN we simply count results for rows returned
    SqliteQueryPtr lastQuery = context->parsedQueries.last();
    if (lastQuery->queryType != SqliteQueryType::Select || lastQuery->explain)
        context->rowsCountingRequired = true;

    if (context->resultsHandler)
    {
        context->resultsHandler(results);
        context->resultsHandler = nullptr;
    }

    context->executionResults = results;
}

void QueryExecutorExecute::handleFailResult(SqlQueryPtr results)
{
    if (!results->isInterrupted())
    {
        context->errorCodeFromSmartExecution = results->getErrorCode();
        context->errorMessageFromSmartExecution = results->getErrorText();
        qWarning() << "Could not execute query with smart method:" << queryExecutor->getOriginalQuery()
                   << "\nError message:" << results->getErrorText()
                   << "\nActual, post-processed query:" << context->processedQuery
                   << "\nSkipping smart execution.";
    }
}

QHash<QString, QVariant> QueryExecutorExecute::getBindParamsForQuery(SqliteQueryPtr query)
{
    QHash<QString, QVariant> queryParams;
    QStringList bindParams = query->tokens.filter(Token::BIND_PARAM).toValueList();
    for (const QString& bindParam : bindParams)
    {
        if (context->queryParameters.contains(bindParam))
            queryParams.insert(bindParam, context->queryParameters[bindParam]);
    }
    return queryParams;
}

bool QueryExecutorExecute::isBeginTransaction(SqliteQueryType queryType)
{
    return (queryType == SqliteQueryType::BeginTrans || queryType == SqliteQueryType::Savepoint);
}

bool QueryExecutorExecute::isCommitTransaction(SqliteQueryType queryType)
{
    return (queryType == SqliteQueryType::CommitTrans || queryType == SqliteQueryType::Release);
}

bool QueryExecutorExecute::isRollbackTransaction(SqliteQueryType queryType)
{
    return queryType == SqliteQueryType::Rollback;
}
