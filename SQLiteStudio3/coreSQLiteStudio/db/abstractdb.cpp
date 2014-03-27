#include "abstractdb.h"
#include "services/dbmanager.h"
#include "common/utils.h"
#include "asyncqueryrunner.h"
#include "sqlresultsrow.h"
#include "common/utils_sql.h"
#include "services/config.h"
#include "sqlerrorresults.h"
#include "sqlerrorcodes.h"
#include "services/notifymanager.h"
#include "log.h"
#include "parser/lexer.h"
#include <QDebug>
#include <QTime>
#include <QWriteLocker>
#include <QReadLocker>
#include <QThreadPool>
#include <QMetaEnum>
#include <QtConcurrent/QtConcurrentRun>

quint32 AbstractDb::asyncId = 1;

AbstractDb::AbstractDb(const QString& name, const QString& path, const QHash<QString, QVariant>& connOptions) :
    name(name), path(path), connOptions(connOptions)
{
}

AbstractDb::~AbstractDb()
{
}

bool AbstractDb::open()
{
    bool res = isOpen() || openQuiet();
    if (res)
        emit connected();

    return res;
}

bool AbstractDb::close()
{
    bool res = !isOpen() || closeQuiet();
    if (res)
        emit disconnected();

    return res;
}

bool AbstractDb::openQuiet()
{
    QWriteLocker locker(&dbOperLock);
    QWriteLocker connectionLocker(&connectionStateLock);
    return openAndSetup();
}

bool AbstractDb::closeQuiet()
{
    QWriteLocker locker(&dbOperLock);
    QWriteLocker connectionLocker(&connectionStateLock);
    interruptExecution();
    bool res = closeInternal();
    clearAttaches();
    registeredFunctions.clear();
    registeredCollations.clear();
    disconnect(FUNCTIONS, SIGNAL(functionListChanged()), this, SLOT(registerAllFunctions()));
    return res;
}

bool AbstractDb::openForProbing()
{
    QWriteLocker locker(&dbOperLock);
    QWriteLocker connectionLocker(&connectionStateLock);
    return openInternal();
}

void AbstractDb::registerAllFunctions()
{
    foreach (const RegisteredFunction& regFn, registeredFunctions)
    {
        if (!deregisterFunction(regFn.name, regFn.argCount))
            qWarning() << "Failed to deregister custom SQL function:" << regFn.name;
    }

    registeredFunctions.clear();

    RegisteredFunction regFn;
    foreach (const FunctionManager::FunctionPtr& fnPtr, FUNCTIONS->getFunctionsForDatabase(getName()))
    {
        regFn.argCount = fnPtr->undefinedArgs ? -1 : fnPtr->arguments.count();
        regFn.name = fnPtr->name;
        regFn.type = fnPtr->type;
        registerFunction(regFn);
    }

    disconnect(FUNCTIONS, SIGNAL(functionListChanged()), this, SLOT(registerAllFunctions()));
    connect(FUNCTIONS, SIGNAL(functionListChanged()), this, SLOT(registerAllFunctions()));
}

void AbstractDb::registerAllCollations()
{
    foreach (const QString& name, registeredCollations)
    {
        if (!deregisterCollation(name))
            qWarning() << "Failed to deregister custom collation:" << name;
    }

    registeredCollations.clear();

    foreach (const CollationManager::CollationPtr& collPtr, COLLATIONS->getCollationsForDatabase(getName()))
        registerCollation(collPtr->name);

    disconnect(COLLATIONS, SIGNAL(collationListChanged()), this, SLOT(registerAllCollations()));
    connect(COLLATIONS, SIGNAL(collationListChanged()), this, SLOT(registerAllCollations()));
}

bool AbstractDb::isOpen()
{
    // We use separate mutex for connection state to avoid situations, when some query is being executed,
    // and we cannot check if database is open, which is not invasive method call.
    QReadLocker connectionLocker(&connectionStateLock);
    return isOpenInternal();
}

QString AbstractDb::generateUniqueDbName(bool lock)
{
    if (lock)
    {
        QReadLocker locker(&dbOperLock);
        return generateUniqueDbNameNoLock();
    }
    else
    {
        return generateUniqueDbNameNoLock();
    }
}

QString AbstractDb::generateUniqueDbNameNoLock()
{
    SqlResultsPtr results = exec("PRAGMA database_list;", AbstractDb::Flag::STRING_REPLACE_ARGS|Flag::NO_LOCK);
    if (results->isError())
    {
        qWarning() << "Could not get PRAGMA database_list. Falling back to internal db list. Error was:" << results->getErrorText();
        lastErrorText = results->getErrorText();
        return generateUniqueName("attached", attachedDbMap.keys());
    }

    QStringList existingDatabases;
    foreach (SqlResultsRowPtr row, results->getAll())
        existingDatabases << row->value("name").toString();

    return generateUniqueName("attached", existingDatabases);
}

ReadWriteLocker::Mode AbstractDb::getLockingMode(const QString &query, Flags flags)
{
    static QStringList readOnlyCommands = {"SELECT", "ANALYZE", "EXPLAIN", "PRAGMA"};

    if (flags.testFlag(Flag::NO_LOCK))
        return ReadWriteLocker::NONE;

    TokenList tokens = Lexer::tokenize(query, getDialect());
    int keywordIdx = tokens.indexOf(Token::KEYWORD);

    if (keywordIdx > -1 && readOnlyCommands.contains(tokens[keywordIdx]->value.toUpper()))
        return ReadWriteLocker::READ;

    return ReadWriteLocker::WRITE;
}

QString AbstractDb::getName()
{
    return name;
}

QString AbstractDb::getPath()
{
    return path;
}

quint8 AbstractDb::getVersion()
{
    return version;
}

Dialect AbstractDb::getDialect()
{
    if (version == 2)
        return Dialect::Sqlite2;
    else
        return Dialect::Sqlite3;
}

QString AbstractDb::getEncoding()
{
    bool doClose = false;
    if (!isOpen())
    {
        if (!openQuiet())
            return QString::null;

        doClose = true;
    }
    QString encoding = exec("PRAGMA encoding;")->getSingleCell().toString();
    if (doClose)
        closeQuiet();

    return encoding;
}

QHash<QString, QVariant>& AbstractDb::getConnectionOptions()
{
    return connOptions;
}

void AbstractDb::setName(const QString& value)
{
    if (isOpen())
    {
        qWarning() << "Tried to change database's name while the database was open.";
        return;
    }
    name = value;
}

void AbstractDb::setPath(const QString& value)
{
    if (isOpen())
    {
        qWarning() << "Tried to change database's file path while the database was open.";
        return;
    }
    path = value;
}

void AbstractDb::setConnectionOptions(const QHash<QString, QVariant>& value)
{
    if (isOpen())
    {
        qWarning() << "Tried to change database's connection options while the database was open.";
        return;
    }
    connOptions = value;
}

SqlResultsPtr AbstractDb::exec(const QString& query, AbstractDb::Flags flags)
{
    return exec(query, QList<QVariant>(), flags);
}

SqlResultsPtr AbstractDb::exec(const QString& query, const QVariant& arg)
{
    return exec(query, {arg});
}

SqlResultsPtr AbstractDb::exec(const QString& query, std::initializer_list<QVariant> argList)
{
    return exec(query, QList<QVariant>(argList));
}

SqlResultsPtr AbstractDb::exec(const QString &query, std::initializer_list<std::pair<QString, QVariant> > argMap)
{
    return exec(query, QHash<QString,QVariant>(argMap));
}

void AbstractDb::asyncExec(const QString &query, const QList<QVariant> &args, AbstractDb::QueryResultsHandler resultsHandler, AbstractDb::Flags flags)
{
    quint32 asyncId = asyncExec(query, args, flags);
    resultHandlers[asyncId] = resultsHandler;
}

void AbstractDb::asyncExec(const QString &query, const QHash<QString, QVariant> &args, AbstractDb::QueryResultsHandler resultsHandler, AbstractDb::Flags flags)
{
    quint32 asyncId = asyncExec(query, args, flags);
    resultHandlers[asyncId] = resultsHandler;
}

void AbstractDb::asyncExec(const QString &query, AbstractDb::QueryResultsHandler resultsHandler, AbstractDb::Flags flags)
{
    quint32 asyncId = asyncExec(query, flags);
    resultHandlers[asyncId] = resultsHandler;
}

SqlResultsPtr AbstractDb::exec(const QString &query, const QList<QVariant>& args, Flags flags)
{
    ReadWriteLocker locker(&dbOperLock, getLockingMode(query, flags));
    return execListArg(query, args, flags);
}

SqlResultsPtr AbstractDb::exec(const QString& query, const QHash<QString, QVariant>& args, AbstractDb::Flags flags)
{
    ReadWriteLocker locker(&dbOperLock, getLockingMode(query, flags));
    return execHashArg(query, args, flags);
}

SqlResultsPtr AbstractDb::execHashArg(const QString& query, const QHash<QString,QVariant>& args, Flags flags)
{
    logSql(this, query, args, flags);
    QString newQuery = query;
    SqlResultsPtr results;
    if (flags.testFlag(AbstractDb::Flag::STRING_REPLACE_ARGS))
    {
        QHashIterator<QString,QVariant> it(args);
        while (it.hasNext())
            newQuery = newQuery.replace(it.key(), it.value().toString());

        results = execInternal(newQuery, QHash<QString,QVariant>());
    }
    else
        results = execInternal(newQuery, args);

    if (flags.testFlag(Flag::PRELOAD))
        results->preload();

    return results;
}

SqlResultsPtr AbstractDb::execListArg(const QString& query, const QList<QVariant>& args, Flags flags)
{
    logSql(this, query, args, flags);
    QString newQuery = query;
    SqlResultsPtr results;
    if (flags.testFlag(AbstractDb::Flag::STRING_REPLACE_ARGS))
    {
        foreach (QVariant arg, args)
            newQuery = newQuery.arg(arg.toString());

        results = execInternal(newQuery, QList<QVariant>());
    }
    else
        results = execInternal(newQuery, args);

    if (flags.testFlag(Flag::PRELOAD))
        results->preload();

    return results;
}

bool AbstractDb::openAndSetup()
{
    bool result = openInternal();
    if (!result)
        return result;

    // Implementation specific initialization
    initAfterOpen();

    // Custom SQL functions
    registerAllFunctions();

    // Custom collations
    registerAllCollations();

    return result;
}

void AbstractDb::initAfterOpen()
{
}

bool AbstractDb::registerCollation(const QString& name)
{
    if (registeredCollations.contains(name))
    {
        qCritical() << "Collation" << name << "is already registered!"
                    << "It should already be deregistered while call to register is being made.";
        return false;
    }

    if (registerCollationInternal(name))
    {
        registeredCollations << name;
        return true;
    }

    qCritical() << "Could not register collation:" << name;
    return false;
}

bool AbstractDb::deregisterCollation(const QString& name)
{
    if (!registeredCollations.contains(name))
    {
        qCritical() << "Collation" << name << "not registered!"
                    << "It should already registered while call to deregister is being made.";
        return false;
    }

    if (deregisterCollationInternal(name))
    {
        registeredCollations.removeOne(name);
        return true;
    }
    qWarning() << "Could not deregister collation:" << name;
    return false;
}

bool AbstractDb::isCollationRegistered(const QString& name)
{
    return registeredCollations.contains(name);
}

QHash<QString, QVariant> AbstractDb::getAggregateContext(void* memPtr)
{
    if (!memPtr)
    {
        qCritical() << "Could not allocate aggregate context.";
        return QHash<QString, QVariant>();
    }

    QHash<QString,QVariant>** aggCtxPtr = reinterpret_cast<QHash<QString,QVariant>**>(memPtr);
    if (!*aggCtxPtr)
        *aggCtxPtr = new QHash<QString,QVariant>();

    return **aggCtxPtr;
}

void AbstractDb::setAggregateContext(void* memPtr, const QHash<QString, QVariant>& aggregateContext)
{
    if (!memPtr)
    {
        qCritical() << "Could not extract aggregate context.";
        return;
    }

    QHash<QString,QVariant>** aggCtxPtr = reinterpret_cast<QHash<QString,QVariant>**>(memPtr);
    **aggCtxPtr = aggregateContext;
}

void AbstractDb::releaseAggregateContext(void* memPtr)
{
    if (!memPtr)
    {
        qCritical() << "Could not release aggregate context.";
        return;
    }

    QHash<QString,QVariant>** aggCtxPtr = reinterpret_cast<QHash<QString,QVariant>**>(memPtr);
    delete *aggCtxPtr;
}

QVariant AbstractDb::evaluateScalar(void* dataPtr, const QList<QVariant>& argList, bool& ok)
{
    if (!dataPtr)
        return QVariant();

    FunctionUserData* userData = reinterpret_cast<FunctionUserData*>(dataPtr);

    return FUNCTIONS->evaluateScalar(userData->name, userData->argCount, argList, userData->db, ok);
}

void AbstractDb::evaluateAggregateStep(void* dataPtr, QHash<QString, QVariant>& aggregateContext, QList<QVariant> argList)
{
    if (!dataPtr)
        return;

    FunctionUserData* userData = reinterpret_cast<FunctionUserData*>(dataPtr);

    QHash<QString,QVariant> storage = aggregateContext["storage"].toHash();
    if (!aggregateContext.contains("initExecuted"))
    {
        FUNCTIONS->evaluateAggregateInitial(userData->name, userData->argCount, userData->db, storage);
        aggregateContext["initExecuted"] = true;
    }

    FUNCTIONS->evaluateAggregateStep(userData->name, userData->argCount, argList, userData->db, storage);
    aggregateContext["storage"] = storage;
}

QVariant AbstractDb::evaluateAggregateFinal(void* dataPtr, QHash<QString, QVariant>& aggregateContext, bool& ok)
{
    if (!dataPtr)
        return QVariant();

    FunctionUserData* userData = reinterpret_cast<FunctionUserData*>(dataPtr);
    QHash<QString,QVariant> storage = aggregateContext["storage"].toHash();

    return FUNCTIONS->evaluateAggregateFinal(userData->name, userData->argCount, userData->db, ok, storage);
}

quint32 AbstractDb::asyncExec(const QString &query, Flags flags)
{
    AsyncQueryRunner* runner = new AsyncQueryRunner(query, QList<QVariant>(), flags);
    return asyncExec(runner);
}

quint32 AbstractDb::asyncExec(const QString& query, const QHash<QString, QVariant>& args, AbstractDb::Flags flags)
{
    AsyncQueryRunner* runner = new AsyncQueryRunner(query, args, flags);
    return asyncExec(runner);
}

quint32 AbstractDb::asyncExec(const QString& query, const QList<QVariant>& args, AbstractDb::Flags flags)
{
    AsyncQueryRunner* runner = new AsyncQueryRunner(query, args, flags);
    return asyncExec(runner);
}

quint32 AbstractDb::asyncExec(AsyncQueryRunner *runner)
{
    quint32 asyncId = generateAsyncId();
    runner->setDb(this);
    runner->setAsyncId(asyncId);

    connect(runner, SIGNAL(finished(AsyncQueryRunner*)),
            this, SLOT(asyncQueryFinished(AsyncQueryRunner*)));

    QThreadPool::globalInstance()->start(runner);

    return asyncId;
}

void AbstractDb::asyncQueryFinished(AsyncQueryRunner *runner)
{
    // Extract everything from the runner
    SqlResultsPtr results = runner->getResults();
    quint32 asyncId = runner->getAsyncId();
    delete runner;

    if (handleResultInternally(asyncId, results))
        return;

    emit asyncExecFinished(asyncId, results);

    if (isReadable() && isWritable())
        emit idle();
}

QString AbstractDb::attach(Db* otherDb)
{
    QWriteLocker locker(&dbOperLock);
    if (!isOpenInternal())
        return QString::null;

    if (attachedDbNameMap.contains(otherDb))
    {
        attachCounter[otherDb]++;
        return attachedDbNameMap[otherDb];
    }

    QString attName = generateUniqueDbName(false);
    SqlResultsPtr results = exec("ATTACH '%1' AS %2;", {otherDb->getPath(), attName}, Flag::STRING_REPLACE_ARGS|Flag::NO_LOCK);
    if (results->isError())
    {
        notifyError(tr("Error attaching database %1: %2").arg(otherDb->getName()).arg(results->getErrorText()));
        lastErrorText = results->getErrorText();
        return QString::null;
    }

    attachedDbMap[attName] = otherDb;
    attachedDbNameMap[otherDb] = attName;

    emit attached(otherDb);
    return attName;
}

void AbstractDb::detach(Db* otherDb)
{
    QWriteLocker locker(&dbOperLock);

    if (!isOpenInternal())
        return;

    detachInternal(otherDb);
}

void AbstractDb::detachInternal(Db* otherDb)
{
    if (!attachedDbNameMap.contains(otherDb))
        return;

    if (attachCounter.contains(otherDb))
    {
        attachCounter[otherDb]--;
        return;
    }

    exec("DETACH %1;", {attachedDbNameMap[otherDb]}, Flag::STRING_REPLACE_ARGS|Flag::NO_LOCK);
    attachedDbMap.remove(attachedDbNameMap[otherDb]);
    attachedDbNameMap.remove(otherDb);
    emit detached(otherDb);
}

void AbstractDb::clearAttaches()
{
    attachedDbMap.clear();
    attachedDbNameMap.clear();
    attachCounter.clear();
}

void AbstractDb::detachAll()
{
    QWriteLocker locker(&dbOperLock);

    if (!isOpenInternal())
        return;

    foreach (Db* db, attachedDbMap.values())
        detachInternal(db);
}

const QHash<Db *, QString> &AbstractDb::getAttachedDatabases()
{
    QReadLocker locker(&dbOperLock);
    return attachedDbNameMap;
}

QSet<QString> AbstractDb::getAllAttaches()
{
    QReadLocker locker(&dbOperLock);
    QSet<QString> attaches = attachedDbMap.keys().toSet();
    // TODO query database for attached databases and unite them here
    return attaches;
}

QString AbstractDb::getUniqueNewObjectName(const QString &attachedDbName)
{
    QString dbName = getPrefixDb(attachedDbName, getDialect());

    QSet<QString> existingNames;
    SqlResultsPtr results = exec("SELECT name FROM %1.sqlite_master", {dbName}, Flag::STRING_REPLACE_ARGS);
    if (results->isError())
        lastErrorText = results->getErrorText();

    foreach (SqlResultsRowPtr row, results->getAll())
        existingNames << row->value(0).toString();

    return randStrNotIn(16, existingNames);
}

QString AbstractDb::getErrorText()
{
    QReadLocker locker(&dbOperLock);
    QString error = getErrorTextInternal();
    if (error.isNull())
        return lastErrorText;

    return error;
}

int AbstractDb::getErrorCode()
{
    QReadLocker locker(&dbOperLock);
    return getErrorCodeInternal();
}

bool AbstractDb::initAfterCreated()
{
    bool isOpenBefore = isOpen();
    if (!isOpenBefore)
    {
        if (!openForProbing())
        {
            qWarning() << "Could not open database for initAfterCreated(). Database:" << name;
            return false;
        }
    }

    // SQLite version
    QVariant value = exec("SELECT sqlite_version()")->getSingleCell();
    version = value.toString().mid(0, 1).toUInt();

    if (!isOpenBefore)
        closeQuiet();

    return true;
}

void AbstractDb::setTimeout(int secs)
{
    timeout = secs;
}

int AbstractDb::getTimeout() const
{
    return timeout;
}

quint32 AbstractDb::generateAsyncId()
{
    if (asyncId > 4000000000)
        asyncId = 1;

    return asyncId++;
}

bool AbstractDb::begin()
{
    QWriteLocker locker(&dbOperLock);

    if (!isOpenInternal())
        return false;

    SqlResultsPtr results = exec("BEGIN;", Flag::NO_LOCK);
    if (results->isError())
    {
        qCritical() << "Error while starting a transaction: " << results->getErrorCode() << results->getErrorText();
        lastErrorText = results->getErrorText();
        return false;
    }

    return true;
}

bool AbstractDb::commit()
{
    QWriteLocker locker(&dbOperLock);

    if (!isOpenInternal())
        return false;

    SqlResultsPtr results = exec("COMMIT;", Flag::NO_LOCK);
    if (results->isError())
    {
        qCritical() << "Error while commiting a transaction: " << results->getErrorCode() << results->getErrorText();
        lastErrorText = results->getErrorText();
        return false;
    }

    return true;
}

bool AbstractDb::rollback()
{
    QWriteLocker locker(&dbOperLock);

    if (!isOpenInternal())
        return false;

    SqlResultsPtr results = exec("ROLLBACK;", Flag::NO_LOCK);
    if (results->isError())
    {
        qCritical() << "Error while rolling back a transaction: " << results->getErrorCode() << results->getErrorText();
        lastErrorText = results->getErrorText();
        return false;
    }

    return true;
}

void AbstractDb::interrupt()
{
    // Lock connection state to forbid closing db before interrupt() returns.
    // This is required by SQLite.
    QWriteLocker locker(&connectionStateLock);
    interruptExecution();
}

void AbstractDb::asyncInterrupt()
{
    QtConcurrent::run(this, &AbstractDb::interrupt);
}

bool AbstractDb::isReadable()
{
    bool res = dbOperLock.tryLockForRead();
    if (res)
        dbOperLock.unlock();

    return res;
}

bool AbstractDb::isWritable()
{
    bool res = dbOperLock.tryLockForWrite();
    if (res)
        dbOperLock.unlock();

    return res;
}

bool AbstractDb::handleResultInternally(quint32 asyncId, SqlResultsPtr results)
{
    if (!resultHandlers.contains(asyncId))
        return false;

    resultHandlers[asyncId](results);
    resultHandlers.remove(asyncId);

    return true;
}

void AbstractDb::registerFunction(const AbstractDb::RegisteredFunction& function)
{
    if (registeredFunctions.contains(function))
    {
        qCritical() << "Function" << function.name << function.argCount << "is already registered!"
                    << "It should already be deregistered while call to register is being made.";
        return;
    }

    bool successful = false;
    switch (function.type)
    {
        case FunctionManager::Function::SCALAR:
            successful = registerScalarFunction(function.name, function.argCount);
            break;
        case FunctionManager::Function::AGGREGATE:
            successful = registerAggregateFunction(function.name, function.argCount);
            break;
    }

    if (successful)
        registeredFunctions << function;
    else
        qCritical() << "Could not register SQL function:" << function.name << function.argCount << function.type;
}

int qHash(const AbstractDb::RegisteredFunction& fn)
{
    return qHash(fn.name) ^ fn.argCount ^ fn.type;
}

bool operator==(const AbstractDb::RegisteredFunction& fn1, const AbstractDb::RegisteredFunction& fn2)
{
    return fn1.name == fn2.name && fn1.argCount == fn2.argCount && fn1.type == fn2.type;
}
