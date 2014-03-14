#ifndef FUNCTIONMANAGERMOCK_H
#define FUNCTIONMANAGERMOCK_H

#include "services/functionmanager.h"

#include <QVariant>

class FunctionManagerMock : public FunctionManager
{
    public:
        void setFunctions(const QList<FunctionPtr>& newFunctions);
        QList<FunctionPtr> getAllFunctions() const;
        QList<FunctionPtr> getFunctionsForDatabase(const QString& dbName) const;
        QVariant evaluateScalar(const QString& name, int argCount, const QList<QVariant>& args, Db* db, bool& ok);
        void evaluateAggregateInitial(const QString& name, int argCount, Db* db, QHash<QString, QVariant>& aggregateStorage);
        void evaluateAggregateStep(const QString& name, int argCount, const QList<QVariant>& args, Db* db, QHash<QString, QVariant>& aggregateStorage);
        QVariant evaluateAggregateFinal(const QString& name, int argCount, Db* db, bool& ok, QHash<QString, QVariant>& aggregateStorage);
};

#endif // FUNCTIONMANAGERMOCK_H
