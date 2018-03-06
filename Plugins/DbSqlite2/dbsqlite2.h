#ifndef DBSQLITE2_H
#define DBSQLITE2_H

#include "dbsqlite2_global.h"
#include "plugins/dbpluginstdfilebase.h"
#include "plugins/genericplugin.h"

class QueryExecutorSqlite2Delete;

class DBSQLITE2SHARED_EXPORT DbSqlite2 : public GenericPlugin, public DbPluginStdFileBase
{
    Q_OBJECT
    SQLITESTUDIO_PLUGIN("dbsqlite2.json")

    public:
        DbSqlite2();

        QString getLabel() const;
        bool checkIfDbServedByPlugin(Db* db) const;
        QList<DbPluginOption> getOptionsList() const;
        bool init();
        void deinit();

    protected:
        Db *newInstance(const QString &name, const QString &path, const QHash<QString, QVariant> &options);

    private:
        QueryExecutorSqlite2Delete* sqlite2DeleteStep = nullptr;
};

#endif // DBSQLITE2_H
