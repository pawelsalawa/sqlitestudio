#include "queryexecutor.h"
#include "sqlerrorresults.h"
#include "sqlerrorcodes.h"
#include "services/dbmanager.h"
#include "db/sqlerrorcodes.h"
#include "services/notifymanager.h"
#include "queryexecutorsteps/queryexecutoraddrowids.h"
#include "queryexecutorsteps/queryexecutorcolumns.h"
#include "queryexecutorsteps/queryexecutorparsequery.h"
#include "queryexecutorsteps/queryexecutorattaches.h"
#include "queryexecutorsteps/queryexecutorcountresults.h"
#include "queryexecutorsteps/queryexecutorexecute.h"
#include "queryexecutorsteps/queryexecutorcellsize.h"
#include "queryexecutorsteps/queryexecutorlimit.h"
#include "queryexecutorsteps/queryexecutororder.h"
#include "queryexecutorsteps/queryexecutorwrapdistinctresults.h"
#include "queryexecutorsteps/queryexecutordatasources.h"
#include "queryexecutorsteps/queryexecutorexplainmode.h"
#include "queryexecutorsteps/queryexecutorreplaceviews.h"
#include "queryexecutorsteps/queryexecutordetectschemaalter.h"
#include "queryexecutorsteps/queryexecutorvaluesmode.h"
#include "common/unused.h"
#include <QMutexLocker>
#include <QDateTime>
#include <QThreadPool>
#include <QDebug>
#include <schemaresolver.h>
#include <parser/lexer.h>
#include <common/table.h>
#include <QtMath>

// TODO modify all executor steps to use rebuildTokensFromContents() method, instead of replacing tokens manually.

QueryExecutor::QueryExecutor(Db* db, const QString& query, QObject *parent) :
    QObject(parent)
{
    context = new Context();
    originalQuery = query;
    setDb(db);
    setAutoDelete(false);

    connect(this, SIGNAL(executionFinished(SqlQueryPtr)), this, SLOT(cleanupAfterExecFinished(SqlQueryPtr)));
    connect(this, SIGNAL(executionFailed(int,QString)), this, SLOT(cleanupAfterExecFailed(int,QString)));
    connect(DBLIST, SIGNAL(dbAboutToBeUnloaded(Db*, DbPlugin*)), this, SLOT(cleanupBeforeDbDestroy(Db*, DbPlugin*)));
}

QueryExecutor::~QueryExecutor()
{
    delete context;
    context = nullptr;
}

void QueryExecutor::setupExecutionChain()
{
    executionChain << new QueryExecutorParseQuery("initial")
                   << new QueryExecutorDetectSchemaAlter()
                   << new QueryExecutorExplainMode()
                   << new QueryExecutorValuesMode()
                   << new QueryExecutorAttaches() // needs to be at the begining, because columns needs to know real databases
                   << new QueryExecutorParseQuery("after Attaches")
                   << new QueryExecutorDataSources()
                   << new QueryExecutorReplaceViews()
                   << new QueryExecutorParseQuery("after ReplaceViews")
                   << new QueryExecutorAddRowIds()
                   << new QueryExecutorParseQuery("after AddRowIds")
                   << new QueryExecutorColumns()
                   << new QueryExecutorParseQuery("after Columns")
                   //<< new QueryExecutorColumnAliases()
                   << new QueryExecutorOrder()
                   << new QueryExecutorWrapDistinctResults()
                   << new QueryExecutorParseQuery("after WrapDistinctResults")
                   << new QueryExecutorCellSize()
                   << new QueryExecutorCountResults()
                   << new QueryExecutorParseQuery("after Order")
                   << new QueryExecutorLimit()
                   << new QueryExecutorParseQuery("after Limit")
                   << new QueryExecutorExecute();

    foreach (QueryExecutorStep* step, executionChain)
        step->init(this, context);
}

void QueryExecutor::clearChain()
{
    foreach (QueryExecutorStep* step, executionChain)
        delete step;

    executionChain.clear();
}

void QueryExecutor::executeChain()
{
    // Go through all remaining steps
    bool result;
    foreach (QueryExecutorStep* currentStep, executionChain)
    {
        if (interrupted)
        {
            stepFailed(currentStep);
            return;
        }

        result = currentStep->exec();
        if (!result)
        {
            stepFailed(currentStep);
            return;
        }
    }

    // We're done.
    clearChain();

    executionMutex.lock();
    executionInProgress = false;
    executionMutex.unlock();

    emit executionFinished(context->executionResults);
}

void QueryExecutor::stepFailed(QueryExecutorStep* currentStep)
{
    qDebug() << "Smart execution failed at step" << currentStep->metaObject()->className() << currentStep->objectName()
             << "\nUsing simple execution method.";

    clearChain();

    if (interrupted)
    {
        executionInProgress = false;
        emit executionFailed(SqlErrorCode::INTERRUPTED, tr("Execution interrupted."));
        return;
    }

    // Clear anything meaningful set up for smart execution - it's not valid anymore and misleads results for simple method
    context->rowIdColumns.clear();

    executeSimpleMethod();
}

void QueryExecutor::cleanupAfterExecFinished(SqlQueryPtr results)
{
    UNUSED(results);
    cleanup();
}

void QueryExecutor::cleanupAfterExecFailed(int code, QString errorMessage)
{
    UNUSED(code);
    UNUSED(errorMessage);
    cleanup();
}

void QueryExecutor::cleanupBeforeDbDestroy(Db* dbToBeUnloaded, DbPlugin* plugin)
{
    UNUSED(plugin);
    if (!dbToBeUnloaded || dbToBeUnloaded != db)
        return;

    setDb(nullptr);
    context->executionResults.clear();
}

void QueryExecutor::setQuery(const QString& query)
{
    originalQuery = query;
}

void QueryExecutor::exec(Db::QueryResultsHandler resultsHandler)
{
    if (!db)
    {
        qWarning() << "Database is not set in QueryExecutor::exec().";
        return;
    }

    if (!db->isOpen())
    {
        error(SqlErrorCode::DB_NOT_OPEN, tr("Database is not open."));
        return;
    }

    // Get exclusive flow for execution on this query executor
    executionMutex.lock();
    if (executionInProgress)
    {
        error(SqlErrorCode::QUERY_EXECUTOR_ERROR, tr("Only one query can be executed simultaneously."));
        executionMutex.unlock();
        return;
    }
    executionInProgress = true;
    executionMutex.unlock();

    this->resultsHandler = resultsHandler;

    if (asyncMode)
        QThreadPool::globalInstance()->start(this);
    else
        run();
}

void QueryExecutor::run()
{
    execInternal();
}

void QueryExecutor::execInternal()
{
    simpleExecution = false;
    interrupted = false;

    if (resultsCountingAsyncId != 0)
    {
        resultsCountingAsyncId = 0;
        db->interrupt();
    }

    // Reset context
    delete context;
    context = new Context();
    context->processedQuery = originalQuery;
    context->explainMode = explainMode;
    context->skipRowCounting = skipRowCounting;
    context->noMetaColumns = noMetaColumns;
    context->resultsHandler = resultsHandler;
    context->preloadResults = preloadResults;

    // Start the execution
    setupExecutionChain();
    executeChain();
}

void QueryExecutor::interrupt()
{
    if (!db)
    {
        qWarning() << "Called interrupt() on empty db in QueryExecutor.";
        return;
    }

    interrupted = true;
    db->asyncInterrupt();
}

void QueryExecutor::countResults()
{
    if (context->skipRowCounting)
        return;

    if (context->countingQuery.isEmpty()) // simple method doesn't provide that
        return;

    if (asyncMode)
    {
        // Start asynchronous results counting query
        resultsCountingAsyncId = db->asyncExec(context->countingQuery, context->queryParameters);
    }
    else
    {
        SqlQueryPtr results = db->exec(context->countingQuery, context->queryParameters);
        context->totalRowsReturned = results->getSingleCell().toLongLong();
        context->totalPages = (int)qCeil(((double)(context->totalRowsReturned)) / ((double)getResultsPerPage()));

        emit resultsCountingFinished(context->rowsAffected, context->totalRowsReturned, context->totalPages);

        if (results->isError())
        {
            notifyError(tr("An error occured while executing the count(*) query, thus data paging will be disabled. Error details from the database: %1")
                        .arg(results->getErrorText()));
        }
    }
}

qint64 QueryExecutor::getLastExecutionTime() const
{
    return context->executionTime;
}

qint64 QueryExecutor::getRowsAffected() const
{
    return context->rowsAffected;
}

qint64 QueryExecutor::getTotalRowsReturned() const
{
    return context->totalRowsReturned;
}

SqliteQueryType QueryExecutor::getExecutedQueryType(int index)
{
    if (context->parsedQueries.size() == 0)
        return SqliteQueryType::UNDEFINED;

    if (index < 0)
        return context->parsedQueries.last()->queryType;

    if (index < context->parsedQueries.size())
        return context->parsedQueries[index]->queryType;

    return SqliteQueryType::UNDEFINED;
}

QSet<QueryExecutor::SourceTablePtr> QueryExecutor::getSourceTables() const
{
    return context->sourceTables;
}

int QueryExecutor::getTotalPages() const
{
    return context->totalPages;
}

QList<QueryExecutor::ResultColumnPtr> QueryExecutor::getResultColumns() const
{
    return context->resultColumns;
}

QList<QueryExecutor::ResultRowIdColumnPtr> QueryExecutor::getRowIdResultColumns() const
{
    return context->rowIdColumns;
}

int QueryExecutor::getMetaColumnCount() const
{
    int count = 0;
    for (ResultRowIdColumnPtr rowIdCol : context->rowIdColumns)
        count += rowIdCol->queryExecutorAliasToColumn.size();

    return count;
}

QSet<QueryExecutor::EditionForbiddenReason> QueryExecutor::getEditionForbiddenGlobalReasons() const
{
    return context->editionForbiddenReasons;
}

void QueryExecutor::setParam(const QString& name, const QVariant& value)
{
    context->queryParameters[name] = value;
}

void QueryExecutor::arg(const QVariant& value)
{
    QVariant::Type type = value.type();
    switch (type)
    {
        case QVariant::Bool:
        case QVariant::Int:
            originalQuery = originalQuery.arg(value.toInt());
            break;
        case QVariant::LongLong:
            originalQuery = originalQuery.arg(value.toLongLong());
            break;
        case QVariant::UInt:
            originalQuery = originalQuery.arg(value.toUInt());
            break;
        case QVariant::ULongLong:
            originalQuery = originalQuery.arg(value.toULongLong());
            break;
        case QVariant::Double:
            originalQuery = originalQuery.arg(value.toDouble());
            break;
        case QVariant::String:
        {
            if (value.canConvert(QVariant::LongLong))
                originalQuery = originalQuery.arg(value.toLongLong());
            else if (value.canConvert(QVariant::Double))
                originalQuery = originalQuery.arg(value.toDouble());
            else
                originalQuery = originalQuery.arg("'"+value.toString().replace("'", "''")+"'");

            break;
        }
        default:
            return;
    }
}

void QueryExecutor::exec(const QString& query)
{
    setQuery(query);
    exec();
}

void QueryExecutor::dbAsyncExecFinished(quint32 asyncId, SqlQueryPtr results)
{
    if (handleRowCountingResults(asyncId, results))
        return;

    if (!simpleExecution)
        return;

    if (this->asyncId == 0)
        return;

    if (this->asyncId != asyncId)
        return;

    this->asyncId = 0;

    simpleExecutionFinished(results);
}

void QueryExecutor::executeSimpleMethod()
{
    simpleExecution = true;
    context->editionForbiddenReasons << EditionForbiddenReason::SMART_EXECUTION_FAILED;
    simpleExecutionStartTime = QDateTime::currentMSecsSinceEpoch();
    asyncId = db->asyncExec(originalQuery, context->queryParameters, Db::Flag::PRELOAD);
}

void QueryExecutor::simpleExecutionFinished(SqlQueryPtr results)
{
    if (results->isError())
    {
        executionMutex.lock();
        executionInProgress = false;
        executionMutex.unlock();
        error(results->getErrorCode(), results->getErrorText());
        return;
    }

    if (simpleExecIsSelect())
        context->countingQuery = "SELECT count(*) AS cnt FROM ("+originalQuery+");";
    else
        context->rowsCountingRequired = true;

    ResultColumnPtr resCol;
    context->resultColumns.clear();
    foreach (const QString& colName, results->getColumnNames())
    {
        resCol = ResultColumnPtr::create();
        resCol->displayName = colName;
        context->resultColumns << resCol;
    }

    context->executionTime = QDateTime::currentMSecsSinceEpoch() - simpleExecutionStartTime;
    context->rowsAffected = results->rowsAffected();
    context->totalRowsReturned = 0;

    executionMutex.lock();
    executionInProgress = false;
    executionMutex.unlock();
    if (context->resultsHandler)
    {
        context->resultsHandler(results);
        context->resultsHandler = nullptr;
    }

    notifyWarn(tr("SQLiteStudio was unable to extract metadata from the query. Results won't be editable."));

    emit executionFinished(results);
}

bool QueryExecutor::simpleExecIsSelect()
{
    TokenList tokens = Lexer::tokenize(originalQuery, db->getDialect());
    tokens.trim();

    // First check if it's explicit "SELECT" or "VALUES" (the latter one added in SQLite 3.8.4).
    TokenPtr token = tokens.first();
    QString upper = token->value.toUpper();
    if (token->type == Token::KEYWORD && (upper == "SELECT" || upper == "VALUES"))
        return true;

    // Now it's only possible to be a SELECT if it starts with "WITH" statement.
    if (token->type != Token::KEYWORD || upper != "WITH")
        return false;

    // Go through all tokens and find which one appears first (exclude contents indise parenthesis,
    // cause there will always be a SELECT for Common Table Expression).
    int depth = 0;
    foreach (token, tokens)
    {
        switch (token->type)
        {
            case Token::PAR_LEFT:
                depth--;
                break;
            case Token::PAR_RIGHT:
                depth++;
                break;
            case Token::KEYWORD:
            {
                if (depth > 0)
                    break;

                upper = token->value.toUpper();
                if (upper == "SELECT")
                    return true;

                if (upper == "UPDATE" || upper == "DELETE" || upper == "INSERT")
                    return false;

                break;
            }
            default:
                break;
        }
    }
    return false;
}

void QueryExecutor::cleanup()
{
    Db* attDb = nullptr;
    foreach (const QString& attDbName, context->dbNameToAttach.leftValues())
    {
        attDb = DBLIST->getByName(attDbName, Qt::CaseInsensitive);
        if (attDbName.isNull())
        {
            qWarning() << "Could not find db by name for cleanup after execution in QueryExecutor. Searched for db named:" << attDbName;
            continue;
        }
        db->detach(attDb);
    }
}

bool QueryExecutor::handleRowCountingResults(quint32 asyncId, SqlQueryPtr results)
{
    if (resultsCountingAsyncId == 0)
        return false;

    if (resultsCountingAsyncId != asyncId)
        return false;

    if (isExecutionInProgress()) // shouldn't be true, but just in case
        return false;

    resultsCountingAsyncId = 0;

    context->totalRowsReturned = results->getSingleCell().toLongLong();
    context->totalPages = (int)qCeil(((double)(context->totalRowsReturned)) / ((double)getResultsPerPage()));

    emit resultsCountingFinished(context->rowsAffected, context->totalRowsReturned, context->totalPages);

    if (results->isError())
    {
        notifyError(tr("An error occured while executing the count(*) query, thus data paging will be disabled. Error details from the database: %1")
                    .arg(results->getErrorText()));
    }

    return true;
}
bool QueryExecutor::getNoMetaColumns() const
{
    return noMetaColumns;
}

void QueryExecutor::setNoMetaColumns(bool value)
{
    noMetaColumns = value;
}

SqlQueryPtr QueryExecutor::getResults() const
{
    return context->executionResults;
}

bool QueryExecutor::wasSchemaModified() const
{
    return context->schemaModified;
}

QList<DataType> QueryExecutor::resolveColumnTypes(Db* db, QList<QueryExecutor::ResultColumnPtr>& columns, bool noDbLocking)
{
    QSet<Table> tables;
    for (ResultColumnPtr col : columns)
        tables << Table(col->database, col->table);

    SchemaResolver resolver(db);
    resolver.setNoDbLocking(noDbLocking);

    QHash<Table,SqliteCreateTablePtr> parsedTables;
    SqliteCreateTablePtr createTable;
    for (const Table& t : tables)
    {
        createTable = resolver.getParsedObject(t.getDatabase(), t.getTable(), SchemaResolver::TABLE).dynamicCast<SqliteCreateTable>();
        if (!createTable)
        {
            qWarning() << "Could not resolve columns of table" << t.getTable() << "while quering datatypes for queryexecutor columns.";
            continue;
        }
        parsedTables[t] = createTable;
    }

    QList<DataType> datatypeList;
    Table t;
    SqliteCreateTable::Column* parsedCol = nullptr;
    for (ResultColumnPtr col : columns)
    {
        t = Table(col->database, col->table);
        if (!parsedTables.contains(t))
        {
            datatypeList << DataType();
            continue;
        }

        parsedCol = parsedTables[t]->getColumn(col->column);
        if (!parsedCol || !parsedCol->type)
        {
            datatypeList << DataType();
            continue;
        }

        datatypeList << parsedCol->type->toDataType();
    }
    return datatypeList;
}

bool QueryExecutor::getAsyncMode() const
{
    return asyncMode;
}

void QueryExecutor::setAsyncMode(bool value)
{
    asyncMode = value;
}

void QueryExecutor::setPreloadResults(bool value)
{
    preloadResults = value;
}

bool QueryExecutor::getExplainMode() const
{
    return explainMode;
}

void QueryExecutor::setExplainMode(bool value)
{
    explainMode = value;
}


void QueryExecutor::error(int code, const QString& text)
{
    emit executionFailed(code, text);
}

Db* QueryExecutor::getDb() const
{
    return db;
}

void QueryExecutor::setDb(Db* value)
{
    if (db)
        disconnect(db, SIGNAL(asyncExecFinished(quint32,SqlQueryPtr)), this, SLOT(dbAsyncExecFinished(quint32,SqlQueryPtr)));

    db = value;

    if (db)
        connect(db, SIGNAL(asyncExecFinished(quint32,SqlQueryPtr)), this, SLOT(dbAsyncExecFinished(quint32,SqlQueryPtr)));
}

bool QueryExecutor::getSkipRowCounting() const
{
    return skipRowCounting;
}

void QueryExecutor::setSkipRowCounting(bool value)
{
    skipRowCounting = value;
}

QString QueryExecutor::getOriginalQuery() const
{
    return originalQuery;
}

int qHash(QueryExecutor::EditionForbiddenReason reason)
{
    return static_cast<int>(reason);
}

int qHash(QueryExecutor::ColumnEditionForbiddenReason reason)
{
    return static_cast<int>(reason);
}

int QueryExecutor::getDataLengthLimit() const
{
    return dataLengthLimit;
}

void QueryExecutor::setDataLengthLimit(int value)
{
    dataLengthLimit = value;
}

bool QueryExecutor::isRowCountingRequired() const
{
    return context->rowsCountingRequired;
}

QString QueryExecutor::getCountingQuery() const
{
    return context->countingQuery;
}

int QueryExecutor::getResultsPerPage() const
{
    return resultsPerPage;
}

void QueryExecutor::setResultsPerPage(int value)
{
    resultsPerPage = value;
}

int QueryExecutor::getPage() const
{
    return page;
}

void QueryExecutor::setPage(int value)
{
    page = value;
}

bool QueryExecutor::isExecutionInProgress()
{
    QMutexLocker executionLock(&executionMutex);
    return executionInProgress;
}

QueryExecutor::Sort::Sort()
{
}

QueryExecutor::Sort::Sort(QueryExecutor::Sort::Order order, int column)
    : order(order), column(column)
{
}

QueryExecutor::Sort::Sort(Qt::SortOrder order, int column)
    : column(column)
{
    switch (order)
    {
        case Qt::AscendingOrder:
            this->order = ASC;
            break;
        case Qt::DescendingOrder:
            this->order = DESC;
            break;
        default:
            this->order = NONE;
            qWarning() << "Invalid sort order passed to QueryExecutor::setSortOrder():" << order;
            break;
    }
}

Qt::SortOrder QueryExecutor::Sort::getQtOrder() const
{
    // The column should be checked first for being > -1.
    if (order == QueryExecutor::Sort::DESC)
        return Qt::DescendingOrder;

    return Qt::AscendingOrder;
}

QueryExecutor::SortList QueryExecutor::getSortOrder() const
{
    return sortOrder;
}

void QueryExecutor::setSortOrder(const SortList& value)
{
    sortOrder = value;
}

int operator==(const QueryExecutor::SourceTable& t1, const QueryExecutor::SourceTable& t2)
{
    return t1.database == t2.database && t1.table == t2.table && t1.alias == t2.alias;
}

int qHash(QueryExecutor::SourceTable sourceTable)
{
    return qHash(sourceTable.database + "." + sourceTable.table + "/" + sourceTable.alias);
}

