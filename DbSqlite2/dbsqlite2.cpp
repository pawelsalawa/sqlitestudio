#include "dbsqlite2.h"
#include "dbsqlite2instance.h"
#include "common/unused.h"
#include <QFileInfo>

DbSqlite2::DbSqlite2()
{
}

Db* DbSqlite2::getInstance(const QString& name, const QString& path, const QHash<QString, QVariant>& options, QString* errorMessage)
{
    UNUSED(errorMessage);
    Db* db = new DbSqlite2Instance(name, path, options);

    if (!db->openForProbing())
    {
        delete db;
        return nullptr;
    }

    SqlResultsPtr results = db->exec("SELECT * FROM sqlite_master");
    if (results->isError())
    {
        delete db;
        return nullptr;
    }

    db->closeQuiet();
    return db;
}

QList<DbPluginOption> DbSqlite2::getOptionsList() const
{
    return QList<DbPluginOption>();
}

QString DbSqlite2::generateDbName(const QVariant& baseValue)
{
    QFileInfo file(baseValue.toString());
    return file.baseName();
}

QString DbSqlite2::getLabel() const
{
    return "SQLite 2";
}

bool DbSqlite2::checkIfDbServedByPlugin(Db* db) const
{
    return (db && dynamic_cast<DbSqlite2Instance*>(db));
}
