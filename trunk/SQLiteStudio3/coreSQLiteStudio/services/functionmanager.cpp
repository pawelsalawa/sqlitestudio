#include "services/functionmanager.h"

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

