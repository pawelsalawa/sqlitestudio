#include "dbsqlite2.h"
#include "dbsqlite2instance.h"

DbSqlite2::DbSqlite2()
{
}

DbQt *DbSqlite2::getInstance()
{
    return new DbSqlite2Instance(getDriver(), getLabel());
}

QString DbSqlite2::getDriver()
{
    return "QSQLITE2";
}


QString DbSqlite2::getLabel() const
{
    return "SQLite2";
}

bool DbSqlite2::checkIfDbServedByPlugin(Db* db) const
{
    return (db && dynamic_cast<DbSqlite2Instance*>(db));
}

