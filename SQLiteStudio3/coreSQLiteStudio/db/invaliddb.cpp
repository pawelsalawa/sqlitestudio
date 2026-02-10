#include "invaliddb.h"
#include <QSet>

InvalidDb::InvalidDb(const QString& name, const QString& path, const QHash<QString, QVariant>& connOptions) :
    name(name), path(path), connOptions(connOptions)
{
}

bool InvalidDb::isOpen()
{
    return false;
}

QString InvalidDb::getName() const
{
    return name;
}

QString InvalidDb::getPath() const
{
    return path;
}

quint8 InvalidDb::getVersion() const
{
    return 0;
}

QString InvalidDb::getEncoding()
{
    return QString();
}

QHash<QString, QVariant>& InvalidDb::getConnectionOptions()
{
    return connOptions;
}

void InvalidDb::setName(const QString& value)
{
    name = value;
}

void InvalidDb::setPath(const QString& value)
{
    path = value;
}

void InvalidDb::setConnectionOptions(const QHash<QString, QVariant>& value)
{
    connOptions = value;
}

void InvalidDb::setTimeout(int secs)
{
    timeout = secs;
}

int InvalidDb::getTimeout() const
{
    return timeout;
}

QList<AliasedColumn> InvalidDb::columnsForQuery(const QString& query)
{
    Q_UNUSED(query);
    return QList<AliasedColumn>();
}

SqlQueryPtr InvalidDb::exec(const QString& query, const QList<QVariant>& args, Db::Flags flags)
{
    Q_UNUSED(query);
    Q_UNUSED(args);
    Q_UNUSED(flags);
    return SqlQueryPtr();
}

SqlQueryPtr InvalidDb::exec(const QString& query, const QHash<QString, QVariant>& args, Db::Flags flags)
{
    Q_UNUSED(query);
    Q_UNUSED(args);
    Q_UNUSED(flags);
    return SqlQueryPtr();
}

SqlQueryPtr InvalidDb::exec(const QString& query, Db::Flags flags)
{
    Q_UNUSED(query);
    Q_UNUSED(flags);
    return SqlQueryPtr();
}

SqlQueryPtr InvalidDb::exec(const QString& query, const QVariant& arg)
{
    Q_UNUSED(query);
    Q_UNUSED(arg);
    return SqlQueryPtr();
}

SqlQueryPtr InvalidDb::exec(const QString& query, std::initializer_list<QVariant> argList)
{
    Q_UNUSED(query);
    Q_UNUSED(argList);
    return SqlQueryPtr();
}

SqlQueryPtr InvalidDb::exec(const QString& query, std::initializer_list<std::pair<QString, QVariant> > argMap)
{
    Q_UNUSED(query);
    Q_UNUSED(argMap);
    return SqlQueryPtr();
}

void InvalidDb::asyncExec(const QString& query, const QList<QVariant>& args, Db::QueryResultsHandler resultsHandler, Db::Flags flags)
{
    Q_UNUSED(query);
    Q_UNUSED(args);
    Q_UNUSED(resultsHandler);
    Q_UNUSED(flags);
}

void InvalidDb::asyncExec(const QString& query, const QHash<QString, QVariant>& args, Db::QueryResultsHandler resultsHandler, Db::Flags flags)
{
    Q_UNUSED(query);
    Q_UNUSED(args);
    Q_UNUSED(resultsHandler);
    Q_UNUSED(flags);
}

void InvalidDb::asyncExec(const QString& query, Db::QueryResultsHandler resultsHandler, Db::Flags flags)
{
    Q_UNUSED(query);
    Q_UNUSED(resultsHandler);
    Q_UNUSED(flags);
}

quint32 InvalidDb::asyncExec(const QString& query, const QList<QVariant>& args, Db::Flags flags)
{
    Q_UNUSED(query);
    Q_UNUSED(args);
    Q_UNUSED(flags);
    return 0;
}

quint32 InvalidDb::asyncExec(const QString& query, const QHash<QString, QVariant>& args, Db::Flags flags)
{
    Q_UNUSED(query);
    Q_UNUSED(args);
    Q_UNUSED(flags);
    return 0;
}

quint32 InvalidDb::asyncExec(const QString& query, Db::Flags flags)
{
    Q_UNUSED(query);
    Q_UNUSED(flags);
    return 0;
}

SqlQueryPtr InvalidDb::prepare(const QString& query)
{
    Q_UNUSED(query);
    return SqlQueryPtr();
}

bool InvalidDb::begin(bool noLock)
{
    Q_UNUSED(noLock);
    return false;
}

bool InvalidDb::commit(bool noLock)
{
    Q_UNUSED(noLock);
    return false;
}

bool InvalidDb::rollback(bool noLock)
{
    Q_UNUSED(noLock);
    return false;
}

void InvalidDb::asyncInterrupt()
{
}

bool InvalidDb::isReadable()
{
    return false;
}

bool InvalidDb::isWritable()
{
    return false;
}

QString InvalidDb::attach(Db* otherDb, bool silent)
{
    Q_UNUSED(otherDb);
    Q_UNUSED(silent);
    return QString();
}

AttachGuard InvalidDb::guardedAttach(Db* otherDb, bool silent)
{
    Q_UNUSED(silent);
    return AttachGuard::create(this, otherDb, QString());
}

void InvalidDb::detach(Db* otherDb)
{
    Q_UNUSED(otherDb);
}

void InvalidDb::detachAll()
{
}

const QHash<Db*, QString>& InvalidDb::getAttachedDatabases()
{
    return attachedDbs;
}

QSet<QString> InvalidDb::getAllAttaches()
{
    return QSet<QString>();
}

QString InvalidDb::getUniqueNewObjectName(const QString& attachedDbName)
{
    Q_UNUSED(attachedDbName);
    return QString();
}

QString InvalidDb::getErrorText()
{
    return QString();
}

int InvalidDb::getErrorCode()
{
    return 0;
}

QString InvalidDb::getTypeLabel() const
{
    return QStringLiteral("INVALID");
}

QString InvalidDb::getTypeClassName() const
{
    return "InvalidDb";
}

bool InvalidDb::initAfterCreated()
{
    return false;
}

bool InvalidDb::deregisterFunction(const QString& name, int argCount)
{
    Q_UNUSED(name);
    Q_UNUSED(argCount);
    return false;
}

bool InvalidDb::registerScalarFunction(const QString& name, int argCount, bool deterministic)
{
    Q_UNUSED(name);
    Q_UNUSED(argCount);
    Q_UNUSED(deterministic);
    return false;
}

bool InvalidDb::registerAggregateFunction(const QString& name, int argCount, bool deterministic)
{
    Q_UNUSED(name);
    Q_UNUSED(argCount);
    Q_UNUSED(deterministic);
    return false;
}

bool InvalidDb::registerCollation(const QString& name)
{
    Q_UNUSED(name);
    return false;
}

bool InvalidDb::deregisterCollation(const QString& name)
{
    Q_UNUSED(name);
    return false;
}

bool InvalidDb::open()
{
    return false;
}

bool InvalidDb::close()
{
    return false;
}

bool InvalidDb::openQuiet()
{
    return false;
}

bool InvalidDb::openForProbing()
{
    return false;
}

bool InvalidDb::closeQuiet()
{
    return false;
}

void InvalidDb::registerUserFunctions()
{
}

void InvalidDb::registerUserCollations()
{
}
QString InvalidDb::getError() const
{
    return error;
}

void InvalidDb::setError(const QString& value)
{
    error = value;
}

bool InvalidDb::loadExtension(const QString& filePath, const QString& initFunc)
{
    Q_UNUSED(filePath);
    Q_UNUSED(initFunc);
    return false;
}

bool InvalidDb::loadExtensionManually(const QString &filePath, const QString &initFunc)
{
    Q_UNUSED(filePath);
    Q_UNUSED(initFunc);
    return false;
}

bool InvalidDb::isComplete(const QString& sql) const
{
    Q_UNUSED(sql);
    return false;
}

Db* InvalidDb::clone() const
{
    return new InvalidDb(name, path, connOptions);
}

bool InvalidDb::isTransactionActive() const
{
    return false;
}

QList<Db::LoadedExtension> InvalidDb::getManuallyLoadedExtensions() const
{
    return {};
}

void InvalidDb::interrupt()
{
}

bool InvalidDb::isValid() const
{
    return false;
}
