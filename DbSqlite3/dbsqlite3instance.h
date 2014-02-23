#ifndef DBSQLITE3INSTANCE_H
#define DBSQLITE3INSTANCE_H

#include "db/dbqt.h"

class DbSqlite3Instance : public DbQt
{
    public:
        DbSqlite3Instance(const QString& driverName, const QString& type);

    protected:
        void interruptExecutionOnHandle(const QVariant& handle);
        void initialDbSetup();
};

#endif // DBSQLITE3INSTANCE_H
