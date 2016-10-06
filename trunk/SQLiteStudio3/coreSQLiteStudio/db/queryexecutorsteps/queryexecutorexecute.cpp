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
//    qDebug() << "q:" << context->processedQuery;

    startTime = QDateTime::currentMSecsSinceEpoch();
    return executeQueries();
}

void QueryExecutorExecute::provideResultColumns(SqlQueryPtr results)
{
    QueryExecutor::ResultColumnPtr resCol;
    foreach (const QString& colName, results->getColumnNames())
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
    int queryCount = context->parsedQueries.size();
    for (const SqliteQueryPtr& query : context->parsedQueries)
    {
        queryStr = query->detokenize();
        bindParamsForQuery = getBindParamsForQuery(query);
        results = db->prepare(queryStr);
        results->setArgs(bindParamsForQuery);
        results->setFlags(flags);

        queryCount--;
        if (queryCount == 0) // last query?
            setupSqlite2ColumnDataTypes(results);

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

void QueryExecutorExecute::setupSqlite2ColumnDataTypes(SqlQueryPtr results)
{
    Sqlite2ColumnDataTypeHelper* sqlite2Helper = dynamic_cast<Sqlite2ColumnDataTypeHelper*>(results.data());
    if (!sqlite2Helper)
        return;

    Table key;
    SqliteCreateTablePtr createTable;

    SchemaResolver resolver(db);
    QHash<Table,SqliteCreateTablePtr> tables;
    for (QueryExecutor::SourceTablePtr tab : context->sourceTables)
    {
        if (tab->table.isNull())
            continue;

        key = Table(tab->database, tab->table);
        createTable = resolver.getParsedObject(tab->database, tab->table, SchemaResolver::TABLE).dynamicCast<SqliteCreateTable>();
        tables[key] = createTable;
    }

    sqlite2Helper->clearBinaryTypes();

    SqliteCreateTable::Column* column = nullptr;
    int idx = -1 + context->rowIdColumns.size();
    for (QueryExecutor::ResultColumnPtr resCol : context->resultColumns)
    {
        idx++;
        key = Table(resCol->database, resCol->table);
        if (!tables.contains(key))
            continue;

        column = tables[key]->getColumn(resCol->column);
        if (column->type && DataType::isBinary(column->type->name))
            sqlite2Helper->setBinaryType(idx);
    }
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
