#include "dbsqlite3.h"
#include "dbsqlite3instance.h"
#include <QSqlQuery>
#include <QSqlDatabase>

DbSqlite3::DbSqlite3()
{
}

DbQt* DbSqlite3::getInstance()
{
    return new DbSqlite3Instance(getDriver(), getLabel());
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
