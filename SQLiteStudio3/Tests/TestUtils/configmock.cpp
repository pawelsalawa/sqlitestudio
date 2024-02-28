#include "configmock.h"

void ConfigMock::init()
{
}

const QString& ConfigMock::getConfigDir()
{
    static const QString cfg;
    return cfg;
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

void ConfigMock::set(const QString&, const QString&, const QVariant&)
{
}

QVariant ConfigMock::get(const QString&, const QString&)
{
    return QVariant();
}

QVariant ConfigMock::get(const QString &, const QString &, const QVariant &)
{
    return QVariant();
}

QHash<QString, QVariant> ConfigMock::getAll()
{
    return QHash<QString, QVariant>();
}

bool ConfigMock::addDb(const QString&, const QString&, const QHash<QString, QVariant>&)
{
    return true;
}

bool ConfigMock::updateDb(const QString&, const QString&, const QString&, const QHash<QString, QVariant>&)
{
    return true;
}

bool ConfigMock::removeDb(const QString&)
{
    return true;
}

bool ConfigMock::isDbInConfig(const QString&)
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

Config::CfgDbPtr ConfigMock::getDb(const QString&)
{
    return Config::CfgDbPtr();
}

void ConfigMock::storeGroups(const QList<Config::DbGroupPtr>&)
{
}

QList<Config::DbGroupPtr> ConfigMock::getGroups()
{
    return QList<Config::DbGroupPtr>();
}

Config::DbGroupPtr ConfigMock::getDbGroup(const QString&)
{
    return Config::DbGroupPtr();
}

qint64 ConfigMock::addSqlHistory(const QString&, const QString&, int, int)
{
    return 0;
}

void ConfigMock::updateSqlHistory(qint64, const QString&, const QString&, int, int)
{
}

void ConfigMock::clearSqlHistory()
{
}

void ConfigMock::deleteSqlHistory(const QList<qint64>&)
{
}

QAbstractItemModel* ConfigMock::getSqlHistoryModel()
{
    return nullptr;
}

void ConfigMock::addCliHistory(const QString&)
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

void ConfigMock::addBindParamHistory(const QVector<QPair<QString, QVariant> >&)
{
}

void ConfigMock::applyBindParamHistoryLimit()
{
}

QVector<QPair<QString, QVariant> > ConfigMock::getBindParamHistory(const QStringList&) const
{
    return QVector<QPair<QString, QVariant>>();
}

void ConfigMock::addPopulateHistory(const QString&, const QString&, int, const QHash<QString, QPair<QString, QVariant> >&)
{
}

void ConfigMock::applyPopulateHistoryLimit()
{
}

QHash<QString, QPair<QString, QVariant>> ConfigMock::getPopulateHistory(const QString&, const QString&, int&) const
{
    return QHash<QString, QPair<QString, QVariant>>();
}

QVariant ConfigMock::getPopulateHistory(const QString&) const
{
    return QVariant();
}

void ConfigMock::addDdlHistory(const QString&, const QString&, const QString&)
{
}

QList<Config::DdlHistoryEntryPtr> ConfigMock::getDdlHistoryFor(const QString&, const QString&, const QDate&)
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

void ConfigMock::begin()
{
}

void ConfigMock::commit()
{
}

void ConfigMock::rollback()
{
}

const QString &ConfigMock::getConfigDir() const
{
    static QString s;
    return s;
}

QString ConfigMock::getConfigFilePath() const
{
    return QString();
}

bool ConfigMock::isMassSaving() const
{
    return false;
}

void ConfigMock::addReportHistory(bool, const QString &, const QString &)
{
}

QList<Config::ReportHistoryEntryPtr> ConfigMock::getReportHistory()
{
    return QList<Config::ReportHistoryEntryPtr>();
}

void ConfigMock::deleteReport(int)
{
}

void ConfigMock::clearReportHistory()
{
}

void ConfigMock::refreshSqlHistory()
{
}

void ConfigMock::refreshDdlHistory()
{
}

QList<QHash<QString, QVariant> > ConfigMock::getScriptFunctions()
{
    return QList<QHash<QString, QVariant> >();
}

void ConfigMock::setScriptFunctions(const QList<QHash<QString, QVariant> >&)
{
}

QString ConfigMock::getSqlite3Version() const
{
    return "3.8.8";
}

bool ConfigMock::isInMemory() const
{
    return true;
}
