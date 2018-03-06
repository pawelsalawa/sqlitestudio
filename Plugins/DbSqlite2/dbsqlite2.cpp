#include "dbsqlite2.h"
#include "dbsqlite2instance.h"
#include "common/unused.h"
#include "db/queryexecutor.h"
#include "queryexecutorsqlite2delete.h"
#include <QFileInfo>

DbSqlite2::DbSqlite2()
{
}

QList<DbPluginOption> DbSqlite2::getOptionsList() const
{
    return QList<DbPluginOption>();
}

bool DbSqlite2::init()
{
    sqlite2DeleteStep = new QueryExecutorSqlite2Delete();
    QueryExecutor::registerStep(QueryExecutor::LAST, sqlite2DeleteStep);
    return true;
}

void DbSqlite2::deinit()
{
    QueryExecutor::deregisterStep(QueryExecutor::LAST, sqlite2DeleteStep);
    safe_delete(sqlite2DeleteStep);
}

Db *DbSqlite2::newInstance(const QString &name, const QString &path, const QHash<QString, QVariant> &options)
{
    return new DbSqlite2Instance(name, path, options);
}

QString DbSqlite2::getLabel() const
{
    return "SQLite 2";
}

bool DbSqlite2::checkIfDbServedByPlugin(Db* db) const
{
    return (db && dynamic_cast<DbSqlite2Instance*>(db));
}
