#include "dbsqlitewx.h"
#include "dbsqlitewxinstance.h"

DbSqliteWx::DbSqliteWx()
{
}

QString DbSqliteWx::getLabel() const
{
    return "WxSQLite3";
}

QList<DbPluginOption> DbSqliteWx::getOptionsList() const
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

bool DbSqliteWx::checkIfDbServedByPlugin(Db *db) const
{
    return (db && dynamic_cast<DbSqliteWxInstance*>(db));
}

Db *DbSqliteWx::newInstance(const QString &name, const QString &path, const QHash<QString, QVariant> &options)
{
    return new DbSqliteWxInstance(name, path, options);
}
