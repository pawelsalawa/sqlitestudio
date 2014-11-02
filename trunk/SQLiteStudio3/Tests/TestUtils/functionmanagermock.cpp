#include "functionmanagermock.h"

void FunctionManagerMock::setScriptFunctions(const QList<ScriptFunction *> &)
{
}

QList<FunctionManager::ScriptFunction *> FunctionManagerMock::getAllScriptFunctions() const
{
    return QList<FunctionManager::ScriptFunction*>();
}

QList<FunctionManager::ScriptFunction *> FunctionManagerMock::getScriptFunctionsForDatabase(const QString&) const
{
    return QList<FunctionManager::ScriptFunction*>();
}

QList<FunctionManager::NativeFunction *> FunctionManagerMock::getAllNativeFunctions() const
{
    return QList<FunctionManager::NativeFunction*>();
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
