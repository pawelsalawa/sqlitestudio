#ifndef DBSQLITE2INSTANCE_H
#define DBSQLITE2INSTANCE_H

#include "db/dbqt.h"

class DbSqlite2Instance : public DbQt
{
    public:
        DbSqlite2Instance(const QString& driverName, const QString& type);

    protected:
        void interruptExecutionOnHandle(const QVariant& handle);
        SqlResultsPtr execInternal(const QString& query, const QList<QVariant>& args);
        SqlResultsPtr execInternal(const QString& query, const QHash<QString,QVariant>& args);
};

#endif // DBSQLITE2INSTANCE_H
