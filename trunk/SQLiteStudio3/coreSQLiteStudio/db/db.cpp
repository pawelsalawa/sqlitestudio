#include "db.h"
#include "dbqt.h"
#include "dbremote.h"
#include "sqlitestudio.h"
#include "dbmanager.h"
#include "utils.h"
#include "asyncqueryrunner.h"
#include "sqlresultsrow.h"
#include "utils_sql.h"
#include "config.h"
#include "sqlerrorresults.h"
#include "sqlerrorcodes.h"
#include "notifymanager.h"
#include "log.h"
#include "parser/lexer.h"
#include <QDebug>
#include <QTime>
#include <QWriteLocker>
#include <QReadLocker>
#include <QThreadPool>
#include <QMetaEnum>
#include <QtConcurrent/QtConcurrentRun>

quint32 Db::asyncId = 1;

Db::Db()
{
}

Db::~Db()
{
}

void Db::metaInit()
{
    qRegisterMetaType<Db*>("Db*");
    qRegisterMetaTypeStreamOperators<Db*>("Db*");
}

QString Db::flagsToString(Db::Flags flags)
{
    int idx = staticMetaObject.indexOfEnumerator("Flag");
    if (idx == -1)
        return QString::null;

    QMetaEnum en = staticMetaObject.enumerator(idx);
    return en.valueToKeys(static_cast<int>(flags));
}

bool Db::open()
{
    bool res = openQuiet();
    if (res)
        emit connected();

    return res;
}

bool Db::close()
{
    bool res = closeQuiet();
    if (res)
        emit disconnected();

    return res;
}

bool Db::openQuiet()
{
    QWriteLocker locker(&dbOperLock);
    QWriteLocker connectionLocker(&connectionStateLock);
    return openAndSetup();
}

bool Db::closeQuiet()
{
    QWriteLocker locker(&dbOperLock);
    QWriteLocker connectionLocker(&connectionStateLock);
    interruptExecution();
    bool res = closeInternal();
    clearAttaches();
    return res;
}

bool Db::isOpen()
{
    // We use separate mutex for connection state to avoid situations, when some query is being executed,
    // and we cannot check if database is open, which is not invasive method call.
    QReadLocker connectionLocker(&connectionStateLock);
    return isOpenInternal();
}

QString Db::generateUniqueDbName(bool lock)
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

QString Db::generateUniqueDbNameNoLock()
{
    SqlResultsPtr results = exec("PRAGMA database_list;", Db::Flag::STRING_REPLACE_ARGS|Flag::NO_LOCK);
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

ReadWriteLocker::Mode Db::getLockingMode(const QString &query, Flags flags)
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

bool Db::init(const QString &name, const QString &path, const QHash<QString, QVariant> &options)
{
    this->name = name;
    this->path = path;
    connOptions = options;
    init();
    return true;
}

QString Db::getName()
{
    return name;
}

QString Db::getPath()
{
    return path;
}

quint8 Db::getVersion()
{
    return version;
}

Dialect Db::getDialect()
{
    if (version == 2)
        return Dialect::Sqlite2;
    else
        return Dialect::Sqlite3;
}

QString Db::getEncoding()
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

QHash<QString, QVariant>& Db::getConnectionOptions()
{
    return connOptions;
}

SqlResultsPtr Db::exec(const QString& query, Db::Flags flags)
{
    return exec(query, QList<QVariant>(), flags);
}

SqlResultsPtr Db::exec(const QString& query, const QVariant& arg)
{
    return exec(query, {arg});
}

SqlResultsPtr Db::exec(const QString& query, std::initializer_list<QVariant> argList)
{
    return exec(query, QList<QVariant>(argList));
}

SqlResultsPtr Db::exec(const QString &query, std::initializer_list<std::pair<QString, QVariant> > argMap)
{
    return exec(query, QHash<QString,QVariant>(argMap));
}

void Db::asyncExec(const QString &query, const QList<QVariant> &args, Db::QueryResultsHandler resultsHandler, Db::Flags flags)
{
    quint32 asyncId = asyncExec(query, args, flags);
    resultHandlers[asyncId] = resultsHandler;
}

void Db::asyncExec(const QString &query, const QHash<QString, QVariant> &args, Db::QueryResultsHandler resultsHandler, Db::Flags flags)
{
    quint32 asyncId = asyncExec(query, args, flags);
    resultHandlers[asyncId] = resultsHandler;
}

void Db::asyncExec(const QString &query, Db::QueryResultsHandler resultsHandler, Db::Flags flags)
{
    quint32 asyncId = asyncExec(query, flags);
    resultHandlers[asyncId] = resultsHandler;
}

SqlResultsPtr Db::exec(const QString &query, const QList<QVariant>& args, Flags flags)
{
    ReadWriteLocker locker(&dbOperLock, getLockingMode(query, flags));
    return execListArg(query, args, flags);
}

SqlResultsPtr Db::exec(const QString& query, const QHash<QString, QVariant>& args, Db::Flags flags)
{
    ReadWriteLocker locker(&dbOperLock, getLockingMode(query, flags));
    return execHashArg(query, args, flags);
}

SqlResultsPtr Db::execHashArg(const QString& query, const QHash<QString,QVariant>& args, Flags flags)
{
    logSql(this, query, args, flags);
    QString newQuery = query;
    SqlResultsPtr results;
    if (flags.testFlag(Db::Flag::STRING_REPLACE_ARGS))
    {
        QHashIterator<QString,QVariant> it(args);
        while (it.hasNext())
            newQuery = newQuery.replace(it.key(), it.value().toString());

        results = execInternal(newQuery, QHash<QString,QVariant>());
    }
    else
        results = execInternal(newQuery, args);

    if (flags.testFlag(Flag::PRELOAD))
    {
        results->preload();
        results->restart();
    }

    return results;
}

SqlResultsPtr Db::execListArg(const QString& query, const QList<QVariant>& args, Flags flags)
{
    logSql(this, query, args, flags);
    QString newQuery = query;
    SqlResultsPtr results;
    if (flags.testFlag(Db::Flag::STRING_REPLACE_ARGS))
    {
        foreach (QVariant arg, args)
            newQuery = newQuery.arg(arg.toString());

        results = execInternal(newQuery, QList<QVariant>());
    }
    else
        results = execInternal(newQuery, args);

    if (flags.testFlag(Flag::PRELOAD))
    {
        results->preload();
        results->restart();
    }

    return results;
}

bool Db::openAndSetup()
{
    bool result = openInternal();
    if (!result)
        return result;

    initialDbSetup();
    return result;
}

void Db::initialDbSetup()
{
}

quint32 Db::asyncExec(const QString &query, Flags flags)
{
    AsyncQueryRunner* runner = new AsyncQueryRunner(query, QList<QVariant>(), flags);
    return asyncExec(runner);
}

quint32 Db::asyncExec(const QString& query, const QHash<QString, QVariant>& args, Db::Flags flags)
{
    AsyncQueryRunner* runner = new AsyncQueryRunner(query, args, flags);
    return asyncExec(runner);
}

quint32 Db::asyncExec(const QString& query, const QList<QVariant>& args, Db::Flags flags)
{
    AsyncQueryRunner* runner = new AsyncQueryRunner(query, args, flags);
    return asyncExec(runner);
}

quint32 Db::asyncExec(AsyncQueryRunner *runner)
{
    quint32 asyncId = generateAsyncId();
    runner->setDb(this);
    runner->setAsyncId(asyncId);

    connect(runner, SIGNAL(finished(AsyncQueryRunner*)),
            this, SLOT(asyncQueryFinished(AsyncQueryRunner*)));

    QThreadPool::globalInstance()->start(runner);

    return asyncId;
}

void Db::asyncQueryFinished(AsyncQueryRunner *runner)
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

QString Db::attach(Db* otherDb)
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

void Db::detach(Db* otherDb)
{
    QWriteLocker locker(&dbOperLock);

    if (!isOpenInternal())
        return;

    detachInternal(otherDb);
}

void Db::detachInternal(Db* otherDb)
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

void Db::clearAttaches()
{
    attachedDbMap.clear();
    attachedDbNameMap.clear();
    attachCounter.clear();
}

void Db::detachAll()
{
    QWriteLocker locker(&dbOperLock);

    if (!isOpenInternal())
        return;

    foreach (Db* db, attachedDbMap.values())
        detachInternal(db);
}

const QHash<Db *, QString> &Db::getAttachedDatabases()
{
    QReadLocker locker(&dbOperLock);
    return attachedDbNameMap;
}

QSet<QString> Db::getAllAttaches()
{
    QReadLocker locker(&dbOperLock);
    QSet<QString> attaches = attachedDbMap.keys().toSet();
    // TODO query database for attached databases and unite them here
    return attaches;
}

QString Db::getUniqueNewObjectName(const QString &attachedDbName)
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

QString Db::getErrorText()
{
    QReadLocker locker(&dbOperLock);
    QString error = getErrorTextInternal();
    if (error.isNull())
        return lastErrorText;

    return error;
}

int Db::getErrorCode()
{
    QReadLocker locker(&dbOperLock);
    return getErrorCodeInternal();
}

quint32 Db::generateAsyncId()
{
    if (asyncId > 4000000000)
        asyncId = 1;

    return asyncId++;
}

bool Db::begin()
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

bool Db::commit()
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

bool Db::rollback()
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

void Db::interrupt()
{
    // Lock connection state to forbid closing db before interrupt() returns.
    // This is required by SQLite.
    QWriteLocker locker(&connectionStateLock);
    interruptExecution();
}

void Db::asyncInterrupt()
{
    QtConcurrent::run(this, &Db::interrupt);
}

bool Db::isReadable()
{
    bool res = dbOperLock.tryLockForRead();
    if (res)
        dbOperLock.unlock();

    return res;
}

bool Db::isWritable()
{
    bool res = dbOperLock.tryLockForWrite();
    if (res)
        dbOperLock.unlock();

    return res;
}

bool Db::handleResultInternally(quint32 asyncId, SqlResultsPtr results)
{
    if (!resultHandlers.contains(asyncId))
        return false;

    resultHandlers[asyncId](results);
    resultHandlers.remove(asyncId);

    return true;
}

QDataStream &operator <<(QDataStream &out, const Db* myObj)
{
    out << reinterpret_cast<quint64>(myObj);
    return out;
}


QDataStream &operator >>(QDataStream &in, Db*& myObj)
{
    quint64 ptr;
    in >> ptr;
    myObj = reinterpret_cast<Db*>(ptr);
    return in;
}
