#include "dbsqlite2instance.h"
#include "parser/lexer.h"
#include "sqlite.h"
#include <QDebug>

DbSqlite2Instance::DbSqlite2Instance(const QString &driverName, const QString &type)
    : DbQt(driverName, type)
{
}

void DbSqlite2Instance::interruptExecutionOnHandle(const QVariant &handle)
{
    if (qstrcmp(handle.typeName(), "sqlite*") != 0)
    {
        qWarning() << "Call to interrupt() on db (DbSqlite2Instance) object, but driver handle is not sqlite*, its:" << handle.typeName();
        return;
    }

    sqlite* sqliteHnd = *static_cast<sqlite**>(const_cast<void*>(handle.data()));
    sqlite_interrupt(sqliteHnd);
}

SqlResultsPtr DbSqlite2Instance::execInternal(const QString& query, const QList<QVariant>& args)
{
    return DbQt::execInternalSqlite2(query, args);
}

SqlResultsPtr DbSqlite2Instance::execInternal(const QString& query, const QHash<QString, QVariant>& args)
{
    return DbQt::execInternalSqlite2(query, args);
}

