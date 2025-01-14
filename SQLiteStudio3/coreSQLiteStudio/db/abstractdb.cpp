#include "abstractdb.h"
#include "services/collationmanager.h"
#include "common/utils.h"
#include "asyncqueryrunner.h"
#include "sqlresultsrow.h"
#include "common/utils_sql.h"
#include "sqlerrorresults.h"
#include "sqlerrorcodes.h"
#include "services/notifymanager.h"
#include "services/sqliteextensionmanager.h"
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
    connect(SQLITESTUDIO, SIGNAL(aboutToQuit()), this, SLOT(appIsAboutToQuit()));
}

AbstractDb::~AbstractDb()
{
    disconnect(SQLITESTUDIO, SIGNAL(aboutToQuit()), this, SLOT(appIsAboutToQuit()));
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
    bool deny = false;
    emit aboutToDisconnect(deny);
    if (deny)
        return false;

    bool open = isOpen();
    if (open)
        flushWal();

    bool res = !open || closeQuiet();
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
    QWriteLocker connectionLocker(&connectionStateLock);
    interruptExecution();

    QWriteLocker locker(&dbOperLock);
    bool res = closeInternal();
    clearAttaches();
    registeredFunctions.clear();
    registeredCollations.clear();
    if (FUNCTIONS) // FUNCTIONS is already null when closing db while closing entire app
        disconnect(FUNCTIONS, SIGNAL(functionListChanged()), this, SLOT(registerUserFunctions()));

    return res;
}

bool AbstractDb::openForProbing()
{
    QWriteLocker locker(&dbOperLock);
    QWriteLocker connectionLocker(&connectionStateLock);
    bool res = openInternal();
    if (!res)
        return res;

    // Implementation specific initialization
    initAfterOpen();
    return res;
}

void AbstractDb::registerUserFunctions()
{
    QMutableSetIterator<RegisteredFunction> it(registeredFunctions);
    while (it.hasNext())
    {
        const RegisteredFunction& regFn = it.next();
        if (regFn.builtIn)
            continue;

        if (!deregisterFunction(regFn.name, regFn.argCount))
            qWarning() << "Failed to deregister custom SQL function:" << regFn.name;

        it.remove();
    }

    registeredFunctions.clear();

    RegisteredFunction regFn;
    for (FunctionManager::ScriptFunction*& fnPtr : FUNCTIONS->getScriptFunctionsForDatabase(getName()))
    {
        regFn.argCount = fnPtr->undefinedArgs ? -1 : fnPtr->arguments.count();
        regFn.name = fnPtr->name;
        regFn.type = fnPtr->type;
        regFn.deterministic = fnPtr->deterministic;
        registerFunction(regFn);
    }
}

void AbstractDb::registerBuiltInFunctions()
{
    RegisteredFunction regFn;
    for (FunctionManager::NativeFunction*& fnPtr : FUNCTIONS->getAllNativeFunctions())
    {
        regFn.argCount = fnPtr->undefinedArgs ? -1 : fnPtr->arguments.count();
        regFn.name = fnPtr->name;
        regFn.type = fnPtr->type;
        regFn.builtIn = true;
        regFn.deterministic = fnPtr->deterministic;
        registerFunction(regFn);
    }
}

void AbstractDb::registerUserCollations()
{
    for (QString& name : registeredCollations)
    {
        if (!deregisterCollation(name))
            qWarning() << "Failed to deregister custom collation:" << name;
    }

    registeredCollations.clear();

    for (CollationManager::CollationPtr& collPtr : COLLATIONS->getCollationsForDatabase(getName()))
        registerCollation(collPtr->name);

    disconnect(COLLATIONS, SIGNAL(collationListChanged()), this, SLOT(registerUserCollations()));
    connect(COLLATIONS, SIGNAL(collationListChanged()), this, SLOT(registerUserCollations()));
}

void AbstractDb::loadExtensions()
{
    for (SqliteExtensionManager::ExtensionPtr& extPtr : SQLITE_EXTENSIONS->getExtensionForDatabase(getName()))
        loadedExtensionCount += loadExtension(extPtr->filePath, extPtr->initFunc) ? 1 : 0;

    connect(SQLITE_EXTENSIONS, SIGNAL(extensionListChanged()), this, SLOT(reloadExtensions()));
}

void AbstractDb::reloadExtensions()
{
    if (!isOpen())
        return;

    bool doOpen = false;
    if (loadedExtensionCount > 0)
    {
        if (!closeQuiet())
        {
            qWarning() << "Failed to close database for extension reloading.";
            return;
        }

        doOpen = true;
        loadedExtensionCount = 0;
        disconnect(SQLITE_EXTENSIONS, SIGNAL(extensionListChanged()), this, SLOT(reloadExtensions()));
    }

    if (doOpen && !openQuiet())
    {
        qCritical() << "Failed to re-open database for extension reloading.";
        return;
    }

    loadExtensions();
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
    SqlQueryPtr results = exec("PRAGMA database_list;", Db::Flag::NO_LOCK);
    if (results->isError())
    {
        qWarning() << "Could not get PRAGMA database_list. Falling back to internal db list. Error was:" << results->getErrorText();
        return generateUniqueName("attached", attachedDbMap.leftValues());
    }

    QStringList existingDatabases;
    for (SqlResultsRowPtr row : results->getAll())
        existingDatabases << row->value("name").toString();

    return generateUniqueName("attached", existingDatabases);
}

ReadWriteLocker::Mode AbstractDb::getLockingMode(const QString &query, Flags flags)
{
    return ReadWriteLocker::getMode(query, flags.testFlag(Flag::NO_LOCK));
}

QString AbstractDb::getName() const
{
    return name;
}

QString AbstractDb::getPath() const
{
    return path;
}

quint8 AbstractDb::getVersion() const
{
    return version;
}

QString AbstractDb::getEncoding()
{
    bool doClose = false;
    if (!isOpen())
    {
        if (!openQuiet())
            return QString();

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

SqlQueryPtr AbstractDb::exec(const QString& query, AbstractDb::Flags flags)
{
    return exec(query, QList<QVariant>(), flags);
}

SqlQueryPtr AbstractDb::exec(const QString& query, const QVariant& arg)
{
    return exec(query, {arg});
}

SqlQueryPtr AbstractDb::exec(const QString& query, std::initializer_list<QVariant> argList)
{
    return exec(query, QList<QVariant>(argList));
}

SqlQueryPtr AbstractDb::exec(const QString &query, std::initializer_list<std::pair<QString, QVariant> > argMap)
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

SqlQueryPtr AbstractDb::exec(const QString &query, const QList<QVariant>& args, Flags flags)
{
    return execListArg(query, args, flags);
}

SqlQueryPtr AbstractDb::exec(const QString& query, const QHash<QString, QVariant>& args, AbstractDb::Flags flags)
{
    return execHashArg(query, args, flags);
}

SqlQueryPtr AbstractDb::execHashArg(const QString& query, const QHash<QString,QVariant>& args, Flags flags)
{
    if (!isOpenInternal())
        return SqlQueryPtr(new SqlErrorResults(SqlErrorCode::DB_NOT_OPEN, tr("Cannot execute query on closed database.")));

    QString newQuery = query;
    SqlQueryPtr queryStmt = prepare(newQuery);
    queryStmt->setArgs(args);
    queryStmt->setFlags(flags);
    queryStmt->execute();

    if (flags.testFlag(Flag::PRELOAD))
        queryStmt->preload();

    return queryStmt;
}

SqlQueryPtr AbstractDb::execListArg(const QString& query, const QList<QVariant>& args, Flags flags)
{
    if (!isOpenInternal())
        return SqlQueryPtr(new SqlErrorResults(SqlErrorCode::DB_NOT_OPEN, tr("Cannot execute query on closed database.")));

    QString newQuery = query;
    SqlQueryPtr queryStmt = prepare(newQuery);
    queryStmt->setArgs(args);
    queryStmt->setFlags(flags);
    queryStmt->execute();

    if (flags.testFlag(Flag::PRELOAD))
        queryStmt->preload();

    return queryStmt;
}

bool AbstractDb::openAndSetup()
{
    bool result = openInternal();
    if (!result)
        return result;

    // When this is an internal configuration database
    if (connOptions.contains(DB_PURE_INIT))
        return true;

    // Implementation specific initialization
    initAfterOpen();

    // Built-in SQL functions
    registerBuiltInFunctions();

    // Load extension
    loadExtensions();

    // Custom SQL functions
    registerUserFunctions();

    // Custom collations
    registerUserCollations();

    disconnect(FUNCTIONS, SIGNAL(functionListChanged()), this, SLOT(registerUserFunctions()));
    connect(FUNCTIONS, SIGNAL(functionListChanged()), this, SLOT(registerUserFunctions()));

    return result;
}

void AbstractDb::initAfterOpen()
{
}

void AbstractDb::checkForDroppedObject(const QString& query)
{
    TokenList tokens = Lexer::tokenize(query);
    tokens.trim(Token::OPERATOR, ";");
    if (tokens.size() == 0)
        return;

    if (tokens[0]->type != Token::KEYWORD || tokens.first()->value.toUpper() != "DROP")
        return;

    tokens.removeFirst(); // remove "DROP" from front
    tokens.trimLeft(); // remove whitespaces and comments from front
    if (tokens.size() == 0)
    {
        qWarning() << "Successful execution of DROP, but after removing DROP from front of the query, nothing has left. Original query:" << query;
        return;
    }

    QString type = tokens.first()->value.toUpper();

    // Now go to the first ID in the tokens
    while (tokens.size() > 0 && tokens.first()->type != Token::OTHER)
        tokens.removeFirst();

    if (tokens.size() == 0)
    {
        qWarning() << "Successful execution of DROP, but after removing DROP and non-ID tokens from front of the query, nothing has left. Original query:" << query;
        return;
    }

    QString database = "main";
    QString object;

    if (tokens.size() > 1)
    {
        database = tokens.first()->value;
        object = tokens.last()->value;
    }
    else
        object = tokens.first()->value;

    object = stripObjName(object);

    if (type == "TABLE")
        emit dbObjectDeleted(database, object, DbObjectType::TABLE);
    else if (type == "INDEX")
        emit dbObjectDeleted(database, object, DbObjectType::INDEX);
    else if (type == "TRIGGER")
        emit dbObjectDeleted(database, object, DbObjectType::TRIGGER);
    else if (type == "VIEW")
        emit dbObjectDeleted(database, object, DbObjectType::VIEW);
    else
        qWarning() << "Unknown object type dropped:" << type;
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

bool AbstractDb::beginNoLock()
{
    if (!isOpenInternal())
        return false;

    SqlQueryPtr results = exec("BEGIN;", Flag::NO_LOCK);
    if (results->isError())
    {
        qCritical() << "Error while starting a transaction: " << results->getErrorCode() << results->getErrorText();
        return false;
    }

    return true;
}

bool AbstractDb::commitNoLock()
{
    if (!isOpenInternal())
        return false;

    SqlQueryPtr results = exec("COMMIT;", Flag::NO_LOCK);
    if (results->isError())
    {
        qCritical() << "Error while committing a transaction: " << results->getErrorCode() << results->getErrorText();
        return false;
    }

    return true;
}

bool AbstractDb::rollbackNoLock()
{
    if (!isOpenInternal())
        return false;

    SqlQueryPtr results = exec("ROLLBACK;", Flag::NO_LOCK);
    if (results->isError())
    {
        qCritical() << "Error while rolling back a transaction: " << results->getErrorCode() << results->getErrorText();
        return false;
    }

    return true;
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
    SqlQueryPtr results = runner->getResults();
    quint32 asyncId = runner->getAsyncId();
    delete runner;

    if (handleResultInternally(asyncId, results))
        return;

    emit asyncExecFinished(asyncId, results);

    if (isReadable() && isWritable())
        emit idle();
}

void AbstractDb::appIsAboutToQuit()
{
    if (isOpen())
        flushWal();
}

QString AbstractDb::attach(Db* otherDb, bool silent)
{
    QWriteLocker locker(&dbOperLock);
    if (!isOpenInternal())
        return QString();

    if (attachedDbMap.containsRight(otherDb))
    {
        attachCounter[otherDb]++;
        return attachedDbMap.valueByRight(otherDb);
    }

    QString attName = generateUniqueDbName(false);
    SqlQueryPtr results = exec(getAttachSql(otherDb, attName), Flag::NO_LOCK);
    if (results->isError())
    {
        if (!silent)
            notifyError(tr("Error attaching database %1: %2").arg(otherDb->getName()).arg(results->getErrorText()));
        else
            qDebug() << QString("Error attaching database %1: %2").arg(otherDb->getName()).arg(results->getErrorText());

        return QString();
    }

    attachedDbMap.insert(attName, otherDb);

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
    if (!attachedDbMap.containsRight(otherDb))
        return;

    if (attachCounter.contains(otherDb))
    {
        attachCounter[otherDb]--;
        return;
    }

    QString dbName = attachedDbMap.valueByRight(otherDb);
    SqlQueryPtr res = exec(QString("DETACH %1;").arg(dbName), Flag::NO_LOCK);
    if (res->isError())
    {
        qCritical() << "Cannot detach" << dbName << " / " << otherDb->getName() << ":" << res->getErrorText();
        return;
    }
    attachedDbMap.removeRight(otherDb);
    emit detached(otherDb);
}

void AbstractDb::clearAttaches()
{
    attachedDbMap.clear();
    attachCounter.clear();
}

void AbstractDb::detachAll()
{
    QWriteLocker locker(&dbOperLock);

    if (!isOpenInternal())
        return;

    for (Db* db : attachedDbMap.rightValues())
        detachInternal(db);
}

const QHash<Db *, QString> &AbstractDb::getAttachedDatabases()
{
    QReadLocker locker(&dbOperLock);
    return attachedDbMap.toInvertedQHash();
}

QSet<QString> AbstractDb::getAllAttaches()
{
    QReadLocker locker(&dbOperLock);
    QSet<QString> attaches = toSet(attachedDbMap.leftValues());
    // TODO query database for attached databases and unite them here
    return attaches;
}

QString AbstractDb::getUniqueNewObjectName(const QString &attachedDbName)
{
    QString dbName = getPrefixDb(attachedDbName);

    QSet<QString> existingNames;
    SqlQueryPtr results = exec(QString("SELECT name FROM %1.sqlite_master").arg(dbName));

    for (SqlResultsRowPtr& row : results->getAll())
        existingNames << row->value(0).toString();

    return randStrNotIn(16, existingNames, false);
}

QString AbstractDb::getErrorText()
{
    QReadLocker locker(&dbOperLock);
    return getErrorTextInternal();
}

int AbstractDb::getErrorCode()
{
    QReadLocker locker(&dbOperLock);
    return getErrorCodeInternal();
}

bool AbstractDb::initAfterCreated()
{
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

bool AbstractDb::isValid() const
{
    return true;
}

QString AbstractDb::getAttachSql(Db* otherDb, const QString& generatedAttachName)
{
    return QString("ATTACH '%1' AS %2;").arg(otherDb->getPath(), generatedAttachName);
}

quint32 AbstractDb::generateAsyncId()
{
    if (asyncId > 4000000000)
        asyncId = 1;

    return asyncId++;
}

bool AbstractDb::begin(bool noLock)
{
    if (noLock)
        return beginNoLock();

    QWriteLocker locker(&dbOperLock);
    return beginNoLock();
}

bool AbstractDb::commit(bool noLock)
{
    if (noLock)
        return commitNoLock();

    QWriteLocker locker(&dbOperLock);
    return commitNoLock();
}

bool AbstractDb::rollback(bool noLock)
{
    if (noLock)
        return rollbackNoLock();

    QWriteLocker locker(&dbOperLock);
    return rollbackNoLock();
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
    QThreadPool::globalInstance()->start([this]() {interrupt();});
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

AttachGuard AbstractDb::guardedAttach(Db* otherDb, bool silent)
{
    QString attachName = attach(otherDb, silent);
    return AttachGuard::create(this, otherDb, attachName);
}

bool AbstractDb::handleResultInternally(quint32 asyncId, SqlQueryPtr results)
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
        return; // native function was overwritten by script function

    bool successful = false;
    switch (function.type)
    {
        case FunctionManager::ScriptFunction::SCALAR:
            successful = registerScalarFunction(function.name, function.argCount, function.deterministic);
            break;
        case FunctionManager::ScriptFunction::AGGREGATE:
            successful = registerAggregateFunction(function.name, function.argCount, function.deterministic);
            break;
    }

    if (successful)
        registeredFunctions << function;
    else
        qCritical() << "Could not register SQL function:" << function.name << function.argCount << function.type;
}

void AbstractDb::flushWal()
{
    if (flushWalInternal())
    {
        if (exec("PRAGMA journal_mode")->getSingleCell().toString() == "wal")
        {
            exec("PRAGMA journal_mode = delete;", Flag::ZERO_TIMEOUT);
            exec("PRAGMA journal_mode = wal;", Flag::ZERO_TIMEOUT);
        }
    }
    else
        notifyWarn(tr("Failed to make full WAL checkpoint on database '%1'. Error returned from SQLite engine: %2").arg(name, getErrorTextInternal()));
}

size_t qHash(const AbstractDb::RegisteredFunction& fn)
{
    return qHash(fn.name) ^ fn.argCount ^ fn.type;
}

bool operator==(const AbstractDb::RegisteredFunction& fn1, const AbstractDb::RegisteredFunction& fn2)
{
    return fn1.name == fn2.name && fn1.argCount == fn2.argCount && fn1.type == fn2.type;
}
