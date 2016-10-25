#ifndef DBSQLITEWX_H
#define DBSQLITEWX_H

#include "plugins/genericplugin.h"
#include "plugins/dbpluginstdfilebase.h"
#include <QObject>

class DbSqliteWx : public GenericPlugin, public DbPluginStdFileBase
{
    Q_OBJECT
    SQLITESTUDIO_PLUGIN("dbsqlitewx.json")

    public:
        DbSqliteWx();

        QString getLabel() const;
        QList<DbPluginOption> getOptionsList() const;
        bool checkIfDbServedByPlugin(Db *db) const;

        static_char* PASSWORD_OPT = "password";

    protected:
        Db *newInstance(const QString &name, const QString &path, const QHash<QString, QVariant> &options);
};

#endif // DBSQLITEWX_H
