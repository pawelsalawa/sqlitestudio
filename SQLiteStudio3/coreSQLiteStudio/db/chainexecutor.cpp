#include "chainexecutor.h"
#include "sqlerrorcodes.h"
#include <QDebug>

ChainExecutor::ChainExecutor(QObject *parent) :
    QObject(parent)
{
}

bool ChainExecutor::getTransaction() const
{
    return transaction;
}

void ChainExecutor::setTransaction(bool value)
{
    transaction = value;
}
QStringList ChainExecutor::getQueries() const
{
    return sqls;
}

void ChainExecutor::setQueries(const QStringList& value)
{
    sqls = value;
    queryParams.clear();
}

void ChainExecutor::exec()
{
    if (!db)
    {
        emit failure(SqlErrorCode::DB_NOT_DEFINED, tr("The database for executing queries was not defined.", "chain executor"));
        return;
    }

    if (!db->isOpen())
    {
        emit failure(SqlErrorCode::DB_NOT_OPEN, tr("The database for executing queries was not open.", "chain executor"));
        return;
    }

    if (transaction && !db->begin())
    {
        emit failure(db->getErrorCode(), tr("Could not start a database transaction. Details: %1", "chain executor").arg(db->getErrorText()));
        return;
    }

    currentSqlIndex = 0;
    if (async)
        executeCurrentSql();
    else
        executeSync();
}

void ChainExecutor::interrupt()
{
    interrupted = true;
    db->interrupt();
}

void ChainExecutor::executeCurrentSql()
{
    if (currentSqlIndex >= sqls.size())
    {
        executionSuccessful();
        return;
    }

    if (interrupted)
    {
        executionFailure(SqlErrorCode::INTERRUPTED, tr("Interrupted", "chain executor"));
        return;
    }

    asyncId = db->asyncExec(sqls[currentSqlIndex], queryParams);
}

QList<bool> ChainExecutor::getMandatoryQueries() const
{
    return mandatoryQueries;
}

void ChainExecutor::setMandatoryQueries(const QList<bool>& value)
{
    mandatoryQueries = value;
}

Db* ChainExecutor::getDb() const
{
    return db;
}

void ChainExecutor::setDb(Db* value)
{
    if (db)
        disconnect(db, SIGNAL(asyncExecFinished(quint32,SqlResultsPtr)), this, SLOT(handleAsyncResults(quint32,SqlResultsPtr)));

    db = value;

    if (db)
        connect(db, SIGNAL(asyncExecFinished(quint32,SqlResultsPtr)), this, SLOT(handleAsyncResults(quint32,SqlResultsPtr)));
}


void ChainExecutor::handleAsyncResults(quint32 asyncId, SqlResultsPtr results)
{
    if (asyncId != this->asyncId)
        return;

    if (!handleResults(results))
        return;

    currentSqlIndex++;
    executeCurrentSql();
}

void ChainExecutor::executionFailure(int errorCode, const QString& errorText)
{
    if (transaction)
        db->rollback();

    successfulExecution = false;
    executionErrors << errorText;
    emit failure(errorCode, errorText);
}

void ChainExecutor::executionSuccessful()
{
    if (transaction && !db->commit())
    {
        executionFailure(db->getErrorCode(), tr("Could not commit a database transaction. Details: %1", "chain executor").arg(db->getErrorText()));
        return;
    }

    successfulExecution = true;
    emit success();
}

void ChainExecutor::executeSync()
{
    SqlResultsPtr results;
    foreach (const QString& sql, sqls)
    {
        results = db->exec(sql, queryParams);
        if (!handleResults(results))
            return;

        currentSqlIndex++;
    }
    executionSuccessful();
}

bool ChainExecutor::handleResults(SqlResultsPtr results)
{
    if (results->isError())
    {
        if (interrupted || currentSqlIndex >= mandatoryQueries.size() || mandatoryQueries[currentSqlIndex])
        {
            executionFailure(results->getErrorCode(), results->getErrorText());
            return false;
        }
    }
    return true;
}
bool ChainExecutor::getSuccessfulExecution() const
{
    return successfulExecution;
}

void ChainExecutor::setParam(const QString &paramName, const QVariant &value)
{
    queryParams[paramName] = value;
}

bool ChainExecutor::getAsync() const
{
    return async;
}

void ChainExecutor::setAsync(bool value)
{
    async = value;
}

QStringList ChainExecutor::getExecutionErrors() const
{
    return executionErrors;
}
