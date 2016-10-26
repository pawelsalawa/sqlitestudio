#include "dbsqlitesystemdata.h"
#include "dbsqlitesystemdatainstance.h"

DbSqliteSystemData::DbSqliteSystemData()
{
}

QString DbSqliteSystemData::getLabel() const
{
    return "System.Data.SQLite";
}

QList<DbPluginOption> DbSqliteSystemData::getOptionsList() const
{
    QList<DbPluginOption> opts;

    DbPluginOption opt;
    opt.type = DbPluginOption::PASSWORD;
    opt.key = PASSWORD_OPT;
    opt.label = tr("Password (key)");
    opt.toolTip = tr("Leave empty to create or connect to decrypted database.");
    opt.placeholderText = tr("Encryption password");
    opts << opt;

    return opts;
}

bool DbSqliteSystemData::checkIfDbServedByPlugin(Db *db) const
{
    return (db && dynamic_cast<DbSqliteSystemDataInstance*>(db));
}

Db *DbSqliteSystemData::newInstance(const QString &name, const QString &path, const QHash<QString, QVariant> &options)
{
    return new DbSqliteSystemDataInstance(name, path, options);
}
