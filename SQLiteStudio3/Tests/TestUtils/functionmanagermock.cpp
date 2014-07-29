#include "functionmanagermock.h"

void FunctionManagerMock::setScriptFunctions(const QList<FunctionManager::FunctionPtr>&)
{
}

QList<FunctionManager::FunctionPtr> FunctionManagerMock::getAllScriptFunctions() const
{
    return QList<FunctionManager::FunctionPtr>();
}

QList<FunctionManager::FunctionPtr> FunctionManagerMock::getScriptFunctionsForDatabase(const QString&) const
{
    return QList<FunctionManager::FunctionPtr>();
}

QVariant FunctionManagerMock::evaluateScalar(const QString&, int, const QList<QVariant>&, Db*, bool&)
{
    return QVariant();
}

void FunctionManagerMock::evaluateAggregateInitial(const QString&, int, Db*, QHash<QString, QVariant>&)
{
}

void FunctionManagerMock::evaluateAggregateStep(const QString&, int, const QList<QVariant>&, Db*, QHash<QString, QVariant>&)
{
}

QVariant FunctionManagerMock::evaluateAggregateFinal(const QString&, int, Db*, bool&, QHash<QString, QVariant>&)
{
    return QVariant();
}
