#include "dbsqlite3mock.h"
#include "common/unused.h"
#include <QDebug>

DbSqlite3Mock::DbSqlite3Mock(const QString &name, const QString &path, const QHash<QString,QVariant> &options)
    : DbQt3<Sqlite3Mock>(name, path, options, "QSQLITE", "SQLite3")
{
}

QString DbSqlite3Mock::getDriver()
{
    return "QSQLITE";
}

void DbSqlite3Mock::interruptExecutionOnHandle(const QVariant &handle)
{
    UNUSED(handle);
}

void DbSqlite3Mock::registerAllFunctions()
{
    AbstractDb::registerAllFunctions();
}
