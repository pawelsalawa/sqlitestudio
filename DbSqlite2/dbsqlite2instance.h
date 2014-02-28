#ifndef DBSQLITE2INSTANCE_H
#define DBSQLITE2INSTANCE_H

#include "db/dbqt2.h"

struct sqlite;
typedef struct sqlite_func sqlite_func;

class DbSqlite2Instance : public DbQt2
{
    public:
        DbSqlite2Instance(const QString& driverName, const QString& type);

    protected:
        void interruptExecutionOnHandle(const QVariant& handle);
        bool deregisterFunction(const QVariant& handle, const QString& name, int argCount);
        bool registerScalarFunction(const QVariant& handle, const QString& name, int argCount);
        bool registerAggregateFunction(const QVariant& handle, const QString& name, int argCount);

    private:
        struct FunctionUserData
        {
            QString name;
            int argCount = 0;
            Db* db = nullptr;
        };

        static void storeResult(sqlite_func* func, const QVariant& result, bool ok);
        static QList<QVariant> getArgs(int argCount, const char** args);
        static void evaluateScalar(sqlite_func* func, int argCount, const char** args);
        static void evaluateAggregateStep(sqlite_func* func, int argCount, const char** args);
        static void evaluateAggregateFinal(sqlite_func* func);
        static void deleteUserData(void* dataPtr);

        sqlite* getHandle(const QVariant& handle);
};

#endif // DBSQLITE2INSTANCE_H
