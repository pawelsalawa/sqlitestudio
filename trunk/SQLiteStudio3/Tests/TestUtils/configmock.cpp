#include "configmock.h"

void ConfigMock::init()
{
}

void ConfigMock::cleanUp()
{
}

const QString& ConfigMock::getConfigDir()
{
    return QString();
}

void ConfigMock::beginMassSave()
{
}

void ConfigMock::commitMassSave()
{
}

void ConfigMock::rollbackMassSave()
{
}

void ConfigMock::set(const QString& group, const QString& key, const QVariant& value)
{
}

QVariant ConfigMock::get(const QString& group, const QString& key)
{
    return QVariant();
}

QHash<QString, QVariant> ConfigMock::getAll()
{
    return QHash<QString, QVariant>();
}

bool ConfigMock::addDb(const QString& name, const QString& path, const QHash<QString, QVariant>& options)
{
    return true;
}

bool ConfigMock::updateDb(const QString& name, const QString& newName, const QString& path, const QHash<QString, QVariant>& options)
{
    return true;
}

bool ConfigMock::removeDb(const QString& name)
{
    return true;
}

bool ConfigMock::isDbInConfig(const QString& name)
{
    return true;
}

QString ConfigMock::getLastErrorString() const
{
    return QString();
}

QList<Config::CfgDbPtr> ConfigMock::dbList()
{
    return QList<Config::CfgDbPtr>();
}

Config::CfgDbPtr ConfigMock::getDb(const QString& dbName)
{
    return Config::CfgDbPtr();
}

void ConfigMock::storeGroups(const QList<Config::DbGroupPtr>& groups)
{
}

QList<Config::DbGroupPtr> ConfigMock::getGroups()
{
    return QList<Config::DbGroupPtr>();
}

Config::DbGroupPtr ConfigMock::getDbGroup(const QString& dbName)
{
    return Config::DbGroupPtr();
}

qint64 ConfigMock::addSqlHistory(const QString& sql, const QString& dbName, int timeSpentMillis, int rowsAffected)
{
    return 0;
}

void ConfigMock::updateSqlHistory(qint64 id, const QString& sql, const QString& dbName, int timeSpentMillis, int rowsAffected)
{
}

void ConfigMock::clearSqlHistory()
{
}

QAbstractItemModel* ConfigMock::getSqlHistoryModel()
{
    return nullptr;
}

void ConfigMock::addCliHistory(const QString& text)
{
}

void ConfigMock::applyCliHistoryLimit()
{
}

void ConfigMock::clearCliHistory()
{
}

QStringList ConfigMock::getCliHistory() const
{
    return QStringList();
}

void ConfigMock::addDdlHistory(const QString& queries, const QString& dbName, const QString& dbFile)
{
}

QList<Config::DdlHistoryEntryPtr> ConfigMock::getDdlHistoryFor(const QString& dbName, const QString& dbFile, const QDate& date)
{
    return QList<Config::DdlHistoryEntryPtr>();
}

DdlHistoryModel* ConfigMock::getDdlHistoryModel()
{
    return nullptr;
}

void ConfigMock::clearDdlHistory()
{
}

bool ConfigMock::setFunctions(const QList<FunctionManager::FunctionPtr>& functions)
{
    return true;
}

QList<FunctionManager::FunctionPtr> ConfigMock::getFunctions() const
{
    return QList<FunctionManager::FunctionPtr>();
}

void ConfigMock::begin()
{
}

void ConfigMock::commit()
{
}

void ConfigMock::rollback()
{
}
