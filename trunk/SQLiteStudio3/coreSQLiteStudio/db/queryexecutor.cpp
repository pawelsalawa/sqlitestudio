#include "queryexecutor.h"
#include "sqlerrorresults.h"
#include "sqlerrorcodes.h"
#include "sqlitestudio.h"
#include "db/dbmanager.h"
#include "db/sqlerrorcodes.h"
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
#include "unused.h"
#include <QMutexLocker>
#include <QDateTime>
#include <QThreadPool>
#include <QDebug>

// TODO modify all executor steps to use rebuildTokensFromContents() method, instead of replacing tokens manually.

QueryExecutor::QueryExecutor(Db* db, const QString& query, QObject *parent) :
    QObject(parent)
{
    context = new Context();
    originalQuery = query;
    setDb(db);
    setAutoDelete(false);

    connect(this, &QueryExecutor::executionFinished, this, &QueryExecutor::cleanupAfterExecFinished);
    connect(this, &QueryExecutor::executionFailed, this, &QueryExecutor::cleanupAfterExecFailed);
}

QueryExecutor::~QueryExecutor()
{
    delete context;
    context = nullptr;
}

void QueryExecutor::setupExecutionChain()
{
    executionChain << new QueryExecutorParseQuery("initial")
                   << new QueryExecutorExplainMode()
                   << new QueryExecutorAttaches() // needs to be at the begining, because columns needs to know real databases
                   << new QueryExecutorParseQuery("after Attaches")
                   << new QueryExecutorDataSources()
                   << new QueryExecutorReplaceViews()
                   << new QueryExecutorParseQuery("after ReplaceViews")
                   << new QueryExecutorAddRowIds()
                   << new QueryExecutorParseQuery("after AddRowIds")
                   << new QueryExecutorColumns()
                   << new QueryExecutorParseQuery("after Columns")
                   << new QueryExecutorWrapDistinctResults()
                   << new QueryExecutorParseQuery("after WrapDistinctResults")
                   << new QueryExecutorCellSize()
                   << new QueryExecutorCountResults()
                   << new QueryExecutorOrder()
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

    executeSimpleMethod();
}

void QueryExecutor::cleanupAfterExecFinished(SqlResultsPtr results)
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

    this->resultsHandler = resultsHandler;
    QThreadPool::globalInstance()->start(this);
}

void QueryExecutor::run()
{
    execInternal();
}

void QueryExecutor::execInternal()
{
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

    // Start asynchronous results counting query
    resultsCountingAsyncId = db->asyncExec(context->countingQuery, context->queryParameters);
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

void QueryExecutor::dbAsyncExecFinished(quint32 asyncId, SqlResultsPtr results)
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

void QueryExecutor::simpleExecutionFinished(SqlResultsPtr results)
{
    if (results->isError())
    {
        executionMutex.lock();
        executionInProgress = false;
        executionMutex.unlock();
        error(results->getErrorCode(), results->getErrorText());
        return;
    }

    ResultColumnPtr resCol = ResultColumnPtr::create();
    context->resultColumns.clear();
    foreach (const QString& colName, results->getColumnNames())
    {
        resCol->displayName = colName;
        context->resultColumns << resCol;
    }

    context->executionTime = QDateTime::currentMSecsSinceEpoch() - simpleExecutionStartTime;
    context->rowsAffected = results->rowsAffected();
    context->totalRowsReturned = results->rowCount();

    executionMutex.lock();
    executionInProgress = false;
    executionMutex.unlock();
    if (context->resultsHandler)
    {
        context->resultsHandler(results);
        context->resultsHandler = nullptr;
    }

    emit executionFinished(results);
    emit resultsCountingFinished(context->rowsAffected, context->totalRowsReturned, context->totalPages);
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

bool QueryExecutor::handleRowCountingResults(quint32 asyncId, SqlResultsPtr results)
{
    if (resultsCountingAsyncId == 0)
        return false;

    if (resultsCountingAsyncId != asyncId)
        return false;

    if (isExecutionInProgress()) // shouldn't be true, but just in case
        return false;

    resultsCountingAsyncId = 0;

    context->totalRowsReturned = results->getSingleCell().toLongLong();
    context->totalPages = (int)ceil(((double)(context->totalRowsReturned)) / ((double)getResultsPerPage()));

    emit resultsCountingFinished(context->rowsAffected, context->totalRowsReturned, context->totalPages);

    return true;
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
        disconnect(db, SIGNAL(asyncExecFinished(quint32,SqlResultsPtr)), this, SLOT(dbAsyncExecFinished(quint32,SqlResultsPtr)));

    db = value;

    if (db)
        connect(db, SIGNAL(asyncExecFinished(quint32,SqlResultsPtr)), this, SLOT(dbAsyncExecFinished(quint32,SqlResultsPtr)));
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

Qt::SortOrder QueryExecutor::Sort::getQtOrder()
{
    // The column should be checked first for being > -1.
    if (order == QueryExecutor::Sort::DESC)
        return Qt::DescendingOrder;

    return Qt::AscendingOrder;
}

QueryExecutor::Sort QueryExecutor::getSortOrder() const
{
    return sortOrder;
}

void QueryExecutor::setSortOrder(const QueryExecutor::Sort& value)
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

