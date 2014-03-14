#ifndef DBSQLITE3INSTANCE_H
#define DBSQLITE3INSTANCE_H

#include "db/dbqt3.h"
#include <QVariant>

struct sqlite3;
struct sqlite3_context;
typedef struct Mem sqlite3_value;

class Sqlite3 {};

class DbSqlite3Instance : public DbQt3<Sqlite3>
{
    public:
        DbSqlite3Instance(const QString& name, const QString& path, const QHash<QString, QVariant>& connOptions, const QString& driverName, const QString& type);
};

#endif // DBSQLITE3INSTANCE_H
