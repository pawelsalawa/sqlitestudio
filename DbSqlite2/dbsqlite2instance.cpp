#include "dbsqlite2instance.h"

DbSqlite2Instance::DbSqlite2Instance(const QString& name, const QString& path, const QHash<QString, QVariant>& connOptions, const QString &driverName, const QString &type)
    : DbQt2<Sqlite2>(name, path, connOptions, driverName, type)
{
}
