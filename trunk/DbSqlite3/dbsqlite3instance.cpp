#include "dbsqlite3instance.h"
#include <sqlite3.h>
#include <QDebug>

DbSqlite3Instance::DbSqlite3Instance(const QString &driverName, const QString &type)
    : DbQt(driverName, type)
{
}

void DbSqlite3Instance::interruptExecutionOnHandle(const QVariant &handle)
{
    if (qstrcmp(handle.typeName(), "sqlite3*") != 0)
    {
        qWarning() << "Call to interrupt() on db (DbSqlite3Instance) object, but driver handle is not sqlite3*, its:" << handle.typeName();
        return;
    }

    sqlite3* sqliteHnd = *static_cast<sqlite3**>(const_cast<void*>(handle.data()));
    sqlite3_interrupt(sqliteHnd);
}

void DbSqlite3Instance::initialDbSetup()
{
    exec("PRAGMA foreign_keys = 1;", Flag::NO_LOCK);
    exec("PRAGMA recursive_triggers = 1;", Flag::NO_LOCK);
}
