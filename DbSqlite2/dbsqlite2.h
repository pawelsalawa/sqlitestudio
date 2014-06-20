#ifndef DBSQLITE2_H
#define DBSQLITE2_H

#include "dbsqlite2_global.h"
#include "plugins/dbplugin.h"
#include "plugins/genericplugin.h"

class DBSQLITE2SHARED_EXPORT DbSqlite2 : public GenericPlugin, public DbPlugin
{
    Q_OBJECT
    SQLITESTUDIO_PLUGIN("dbsqlite2.json")

    public:
        DbSqlite2();

        QString getLabel() const;
        bool checkIfDbServedByPlugin(Db* db) const;
        Db* getInstance(const QString& name, const QString& path, const QHash<QString, QVariant>& options, QString* errorMessage);
        QList<DbPluginOption> getOptionsList() const;
        QString generateDbName(const QVariant& baseValue);
};

#endif // DBSQLITE2_H
