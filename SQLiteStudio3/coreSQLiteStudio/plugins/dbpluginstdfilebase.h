#ifndef DBPLUGINSTDFILEBASE_H
#define DBPLUGINSTDFILEBASE_H

#include "dbplugin.h"

class API_EXPORT DbPluginStdFileBase : public DbPlugin
{
    public:
        Db *getInstance(const QString &name, const QString &path, const QHash<QString, QVariant> &options, QString *errorMessage);
        QString generateDbName(const QVariant &baseValue);

    protected:
        virtual Db *newInstance(const QString &name, const QString &path, const QHash<QString, QVariant> &options) = 0;
};

#endif // DBPLUGINSTDFILEBASE_H
