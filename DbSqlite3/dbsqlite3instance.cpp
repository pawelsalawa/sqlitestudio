#include "dbsqlite3instance.h"

DbSqlite3Instance::DbSqlite3Instance(const QString& name, const QString& path, const QHash<QString, QVariant>& connOptions, const QString &driverName, const QString &type)
    : DbQt3<Sqlite3>(name, path, connOptions, driverName, type)
{
}
