#include "functionmanagermock.h"

void FunctionManagerMock::setFunctions(const QList<FunctionManager::FunctionPtr>& newFunctions)
{
}

QList<FunctionManager::FunctionPtr> FunctionManagerMock::getAllFunctions() const
{
    return QList<FunctionManager::FunctionPtr>();
}

QList<FunctionManager::FunctionPtr> FunctionManagerMock::getFunctionsForDatabase(const QString& dbName) const
{
    return QList<FunctionManager::FunctionPtr>();
}

QVariant FunctionManagerMock::evaluateScalar(const QString& name, int argCount, const QList<QVariant>& args, Db* db, bool& ok)
{
    return QVariant();
}

void FunctionManagerMock::evaluateAggregateInitial(const QString& name, int argCount, Db* db, QHash<QString, QVariant>& aggregateStorage)
{
}

void FunctionManagerMock::evaluateAggregateStep(const QString& name, int argCount, const QList<QVariant>& args, Db* db, QHash<QString, QVariant>& aggregateStorage)
{
}

QVariant FunctionManagerMock::evaluateAggregateFinal(const QString& name, int argCount, Db* db, bool& ok, QHash<QString, QVariant>& aggregateStorage)
{
    return QVariant();
}
