#ifndef DBADIANTUM_H
#define DBADIANTUM_H

#include "plugins/genericplugin.h"
#include "plugins/dbpluginstdfilebase.h"
#include "dbadiantum_global.h"
#include <QObject>

class DBADIANTUMSHARED_EXPORT DbAdiantum : public GenericPlugin, public DbPluginStdFileBase
{
    Q_OBJECT
    SQLITESTUDIO_PLUGIN("dbadiantum.json")

    public:
        DbAdiantum();

        QString getLabel() const;
        QList<DbPluginOption> getOptionsList() const;
        bool checkIfDbServedByPlugin(Db* db) const;
        bool init();
        void deinit();

        static const char* HEXKEY_OPT;
        static const char* PLAIN_OPT;
        static const char* PRAGMAS_OPT;

    protected:
        Db* newInstance(const QString& name, const QString& path, const QHash<QString, QVariant>& options);
};

#endif // DBADIANTUM_H
