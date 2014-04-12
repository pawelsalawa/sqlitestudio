#include "dbsqlite3.h"

DbSqlite3::DbSqlite3(const QString& name, const QString& path, const QHash<QString, QVariant>& connOptions) :
    AbstractDb3(name, path, connOptions)
{
}

DbSqlite3::DbSqlite3(const QString& name, const QString& path) :
    DbSqlite3(name, path, QHash<QString,QVariant>())
{
}
