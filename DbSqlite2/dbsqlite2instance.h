#ifndef DBSQLITE2INSTANCE_H
#define DBSQLITE2INSTANCE_H

#include "db/dbqt2.h"

struct sqlite;
typedef struct sqlite_func sqlite_func;

class Sqlite2 {};

class DbSqlite2Instance : public DbQt2<Sqlite2>
{
    public:
        DbSqlite2Instance(const QString& driverName, const QString& type);
};

#endif // DBSQLITE2INSTANCE_H
