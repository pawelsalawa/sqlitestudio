#include "dbsqlite2.h"
#include "dbsqlite2instance.h"
#include "common/unused.h"
#include <QFileInfo>

DbSqlite2::DbSqlite2()
{
}

QList<DbPluginOption> DbSqlite2::getOptionsList() const
{
    return QList<DbPluginOption>();
}

Db *DbSqlite2::newInstance(const QString &name, const QString &path, const QHash<QString, QVariant> &options)
{
    return new DbSqlite2Instance(name, path, options);
}

QString DbSqlite2::getLabel() const
{
    return "SQLite 2";
}

bool DbSqlite2::checkIfDbServedByPlugin(Db* db) const
{
    return (db && dynamic_cast<DbSqlite2Instance*>(db));
}
