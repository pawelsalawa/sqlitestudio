#include "dbsqlite3instance.h"

DbSqlite3Instance::DbSqlite3Instance(const QString &driverName, const QString &type)
    : DbQt3<Sqlite3>(driverName, type)
{
}
