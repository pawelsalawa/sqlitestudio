#include "dbpluginsqlite3.h"
#include "db/dbsqlite3.h"
#include "common/unused.h"
#include <QFileInfo>

Db* DbPluginSqlite3::getInstance(const QString& name, const QString& path, const QHash<QString, QVariant>& options, QString* errorMessage)
{
    UNUSED(errorMessage);
    Db* db = new DbSqlite3(name, path, options);

    if (!db->openForProbing())
    {
        delete db;
        return nullptr;
    }

    SqlQueryPtr results = db->exec("SELECT * FROM sqlite_master");
    if (results->isError())
    {
        delete db;
        return nullptr;
    }

    db->closeQuiet();
    return db;
}

QString DbPluginSqlite3::getLabel() const
{
    return "SQLite 3";
}

QList<DbPluginOption> DbPluginSqlite3::getOptionsList() const
{
    return QList<DbPluginOption>();
}

QString DbPluginSqlite3::generateDbName(const QVariant& baseValue)
{
    QFileInfo file(baseValue.toString());
    return file.completeBaseName();
}

bool DbPluginSqlite3::checkIfDbServedByPlugin(Db* db) const
{
    return (db && dynamic_cast<DbSqlite3*>(db));
}
