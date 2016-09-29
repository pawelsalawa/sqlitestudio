#include "chainexecutor.h"
#include "sqlerrorcodes.h"
#include "db/sqlquery.h"
#include <QDebug>
#include <QDateTime>

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
        emit finished(SqlQueryPtr());
        emit failure(SqlErrorCode::DB_NOT_DEFINED, tr("The database for executing queries was not defined.", "chain executor"));
        return;
    }

    if (!db->isOpen())
    {
        emit finished(SqlQueryPtr());
        emit failure(SqlErrorCode::DB_NOT_OPEN, tr("The database for executing queries was not open.", "chain executor"));
        return;
    }

    if (disableForeignKeys && db->getDialect() == Dialect::Sqlite3)
    {
        SqlQueryPtr result = db->exec("PRAGMA foreign_keys = 0;");
        if (result->isError())
        {
            emit finished(SqlQueryPtr());
            emit failure(db->getErrorCode(), tr("Could not disable foreign keys in the database. Details: %1", "chain executor").arg(db->getErrorText()));
            return;
        }
    }

    if (transaction && !db->begin())
    {
        emit finished(SqlQueryPtr());
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
        executionSuccessful(lastExecutionResults);
        return;
    }

    if (interrupted)
    {
        executionFailure(SqlErrorCode::INTERRUPTED, tr("Interrupted", "chain executor"));
        return;
    }

    asyncId = db->asyncExec(sqls[currentSqlIndex], queryParams, getExecFlags());
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
        disconnect(db, SIGNAL(asyncExecFinished(quint32,SqlQueryPtr)), this, SLOT(handleAsyncResults(quint32,SqlQueryPtr)));

    db = value;

    if (db)
        connect(db, SIGNAL(asyncExecFinished(quint32,SqlQueryPtr)), this, SLOT(handleAsyncResults(quint32,SqlQueryPtr)));
}


void ChainExecutor::handleAsyncResults(quint32 asyncId, SqlQueryPtr results)
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

    restoreFk();
    successfulExecution = false;
    executionErrors << ExecutionError(errorCode, errorText);
    emit finished(lastExecutionResults);
    emit failure(errorCode, errorText);
}

void ChainExecutor::executionSuccessful(SqlQueryPtr results)
{
    if (transaction && !db->commit())
    {
        executionFailure(db->getErrorCode(), tr("Could not commit a database transaction. Details: %1", "chain executor").arg(db->getErrorText()));
        return;
    }

    restoreFk();
    successfulExecution = true;
    emit finished(results);
    emit success(results);
}

void ChainExecutor::executeSync()
{
    Db::Flags flags = getExecFlags();
    SqlQueryPtr results;
    for (const QString& sql : sqls)
    {
        results = db->exec(sql, queryParams, flags);
        if (!handleResults(results))
            return;

        currentSqlIndex++;
    }
    executionSuccessful(results);
}

bool ChainExecutor::handleResults(SqlQueryPtr results)
{
    lastExecutionResults = results;
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

Db::Flags ChainExecutor::getExecFlags() const
{
    Db::Flags flags;
    if (disableObjectDropsDetection)
        flags |= Db::Flag::SKIP_DROP_DETECTION;

    return flags;
}

void ChainExecutor::restoreFk()
{
    if (disableForeignKeys && db->getDialect() == Dialect::Sqlite3)
    {
        SqlQueryPtr result = db->exec("PRAGMA foreign_keys = 1;");
        if (result->isError())
            qCritical() << "Could not restore foreign keys in the database after chain execution. Details:" << db->getErrorText();
    }
}

bool ChainExecutor::getDisableObjectDropsDetection() const
{
    return disableObjectDropsDetection;
}

void ChainExecutor::setDisableObjectDropsDetection(bool value)
{
    disableObjectDropsDetection = value;
}

bool ChainExecutor::getDisableForeignKeys() const
{
    return disableForeignKeys;
}

void ChainExecutor::setDisableForeignKeys(bool value)
{
    disableForeignKeys = value;
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

QStringList ChainExecutor::getErrorsMessages() const
{
    QStringList msgs;
    for (const ExecutionError& e : executionErrors)
        msgs << e.second;

    return msgs;
}

const QList<ChainExecutor::ExecutionError>& ChainExecutor::getErrors() const
{
    return executionErrors;
}
