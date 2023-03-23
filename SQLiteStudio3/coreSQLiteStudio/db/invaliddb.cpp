#include "invaliddb.h"
#include "common/unused.h"
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
    UNUSED(query);
    return QList<AliasedColumn>();
}

SqlQueryPtr InvalidDb::exec(const QString& query, const QList<QVariant>& args, Db::Flags flags)
{
    UNUSED(query);
    UNUSED(args);
    UNUSED(flags);
    return SqlQueryPtr();
}

SqlQueryPtr InvalidDb::exec(const QString& query, const QHash<QString, QVariant>& args, Db::Flags flags)
{
    UNUSED(query);
    UNUSED(args);
    UNUSED(flags);
    return SqlQueryPtr();
}

SqlQueryPtr InvalidDb::exec(const QString& query, Db::Flags flags)
{
    UNUSED(query);
    UNUSED(flags);
    return SqlQueryPtr();
}

SqlQueryPtr InvalidDb::exec(const QString& query, const QVariant& arg)
{
    UNUSED(query);
    UNUSED(arg);
    return SqlQueryPtr();
}

SqlQueryPtr InvalidDb::exec(const QString& query, std::initializer_list<QVariant> argList)
{
    UNUSED(query);
    UNUSED(argList);
    return SqlQueryPtr();
}

SqlQueryPtr InvalidDb::exec(const QString& query, std::initializer_list<std::pair<QString, QVariant> > argMap)
{
    UNUSED(query);
    UNUSED(argMap);
    return SqlQueryPtr();
}

void InvalidDb::asyncExec(const QString& query, const QList<QVariant>& args, Db::QueryResultsHandler resultsHandler, Db::Flags flags)
{
    UNUSED(query);
    UNUSED(args);
    UNUSED(resultsHandler);
    UNUSED(flags);
}

void InvalidDb::asyncExec(const QString& query, const QHash<QString, QVariant>& args, Db::QueryResultsHandler resultsHandler, Db::Flags flags)
{
    UNUSED(query);
    UNUSED(args);
    UNUSED(resultsHandler);
    UNUSED(flags);
}

void InvalidDb::asyncExec(const QString& query, Db::QueryResultsHandler resultsHandler, Db::Flags flags)
{
    UNUSED(query);
    UNUSED(resultsHandler);
    UNUSED(flags);
}

quint32 InvalidDb::asyncExec(const QString& query, const QList<QVariant>& args, Db::Flags flags)
{
    UNUSED(query);
    UNUSED(args);
    UNUSED(flags);
    return 0;
}

quint32 InvalidDb::asyncExec(const QString& query, const QHash<QString, QVariant>& args, Db::Flags flags)
{
    UNUSED(query);
    UNUSED(args);
    UNUSED(flags);
    return 0;
}

quint32 InvalidDb::asyncExec(const QString& query, Db::Flags flags)
{
    UNUSED(query);
    UNUSED(flags);
    return 0;
}

SqlQueryPtr InvalidDb::prepare(const QString& query)
{
    UNUSED(query);
    return SqlQueryPtr();
}

bool InvalidDb::begin()
{
    return false;
}

bool InvalidDb::commit()
{
    return false;
}

bool InvalidDb::rollback()
{
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
    UNUSED(otherDb);
    UNUSED(silent);
    return QString();
}

AttachGuard InvalidDb::guardedAttach(Db* otherDb, bool silent)
{
    UNUSED(silent);
    return AttachGuard::create(this, otherDb, QString());
}

void InvalidDb::detach(Db* otherDb)
{
    UNUSED(otherDb);
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
    UNUSED(attachedDbName);
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
    UNUSED(name);
    UNUSED(argCount);
    return false;
}

bool InvalidDb::registerScalarFunction(const QString& name, int argCount, bool deterministic)
{
    UNUSED(name);
    UNUSED(argCount);
    UNUSED(deterministic);
    return false;
}

bool InvalidDb::registerAggregateFunction(const QString& name, int argCount, bool deterministic)
{
    UNUSED(name);
    UNUSED(argCount);
    UNUSED(deterministic);
    return false;
}

bool InvalidDb::registerCollation(const QString& name)
{
    UNUSED(name);
    return false;
}

bool InvalidDb::deregisterCollation(const QString& name)
{
    UNUSED(name);
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
    UNUSED(filePath);
    UNUSED(initFunc);
    return false;
}

bool InvalidDb::isComplete(const QString& sql) const
{
    UNUSED(sql);
    return false;
}

Db* InvalidDb::clone() const
{
    return new InvalidDb(name, path, connOptions);
}

void InvalidDb::interrupt()
{
}

bool InvalidDb::isValid() const
{
    return false;
}
