#include "dbsqlite3.h"
#include "dbsqlite3instance.h"
#include <QSqlQuery>
#include <QSqlDatabase>

DbSqlite3::DbSqlite3()
{
}

DbQt* DbSqlite3::getInstance(const QString& name, const QString& path, const QHash<QString, QVariant>& options)
{
    return new DbSqlite3Instance(name, path, options, getDriver(), getLabel());
}

QString DbSqlite3::getDriver()
{
    return "QSQLITE";
}


QString DbSqlite3::getLabel() const
{
    return "SQLite3";
}

bool DbSqlite3::checkIfDbServedByPlugin(Db* db) const
{
    return (db && dynamic_cast<DbSqlite3Instance*>(db));
}
