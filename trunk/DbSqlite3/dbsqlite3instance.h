#ifndef DBSQLITE3INSTANCE_H
#define DBSQLITE3INSTANCE_H

#include "db/dbqt3.h"
#include <QVariant>

struct sqlite3;
struct sqlite3_context;
typedef struct Mem sqlite3_value;

class DbSqlite3Instance : public DbQt3
{
    public:
        DbSqlite3Instance(const QString& driverName, const QString& type);

    protected:
        void interruptExecutionOnHandle(const QVariant& handle);
        bool deregisterFunction(const QVariant& handle, const QString& name, int argCount);
        bool registerScalarFunction(const QVariant& handle, const QString& name, int argCount);
        bool registerAggregateFunction(const QVariant& handle, const QString& name, int argCount);
        void initialDbSetup();

    private:
        struct FunctionUserData
        {
            QString name;
            int argCount = 0;
            Db* db = nullptr;
        };

        static void storeResult(sqlite3_context* context, const QVariant& result, bool ok);
        static QList<QVariant> getArgs(int argCount, sqlite3_value** args);
        static void evaluateScalar(sqlite3_context* context, int argCount, sqlite3_value** args);
        static void evaluateAggregateStep(sqlite3_context* context, int argCount, sqlite3_value** args);
        static void evaluateAggregateFinal(sqlite3_context* context);
        static void deleteUserData(void* dataPtr);

        sqlite3* getHandle(const QVariant& handle);
};

#endif // DBSQLITE3INSTANCE_H
