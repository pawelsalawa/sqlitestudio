#include "functionmanager.h"
#include "notifymanager.h"
#include "config.h"

FunctionManager::FunctionManager()
{
    init();
}

void FunctionManager::setFunctions(const QList<FunctionManager::FunctionPtr>& newFunctions)
{
    functions = newFunctions;
    refreshFunctionsByName();
    storeInConfig();
    emit functionListChanged();
}

QList<FunctionManager::FunctionPtr> FunctionManager::getAllFunctions() const
{
    return functions;
}

QList<FunctionManager::FunctionPtr> FunctionManager::getFunctionsForDatabase(const QString& dbName) const
{
    QList<FunctionPtr> results;
    foreach (const FunctionPtr& func, functions)
    {
        if (func->allDatabases || func->databases.contains(dbName, Qt::CaseInsensitive))
            results << func;
    }
    return results;
}

FunctionManager::FunctionPtr FunctionManager::getFunction(const QString& name)
{
    if (!functionsByName.contains(name, Qt::CaseInsensitive))
        return FunctionPtr();

    return functionsByName[name];
}

QVariant FunctionManager::evaluateScalar(const QString& name, int argCount, const QList<QVariant>& args, Db* db, bool& ok)
{

}

void FunctionManager::evaluateAggregateStep(const QString& name, int argCount, const QList<QVariant>& args, Db* db)
{

}

QVariant FunctionManager::evaluateAggregateFinal(const QString& name, int argCount, Db* db, bool& ok)
{

}

QString FunctionManager::Function::typeString(Type type)
{
    switch (type)
    {
        case Function::SCALAR:
            return "SCALAR";
        case Function::AGGREGATE:
            return "AGGREGATE";
    }
    return QString::null;
}

FunctionManager::Function::Type FunctionManager::Function::typeString(const QString& type)
{
    if (type == "SCALAR")
        return Function::SCALAR;

    if (type == "AGGREGATE")
        return Function::AGGREGATE;

    return Function::SCALAR;
}

void FunctionManager::init()
{
    functions = CFG->getFunctions();
    refreshFunctionsByName();
}

void FunctionManager::refreshFunctionsByName()
{
    functionsByName.clear();
    foreach (const FunctionPtr& func, functions)
        functionsByName[func->name] = func;
}

void FunctionManager::storeInConfig()
{
    if (!CFG->setFunctions(functions))
    {
        notifyWarn(tr("Could not store custom SQL functions in configuration file. "
                         "You can try editing functions and save them again. "
                         "Otherwise all modifications will be lost after application restart. "
                         "Error details: %1").arg(CFG->getLastErrorString()));
    }
}

FunctionManager::Function::Function()
{
}
