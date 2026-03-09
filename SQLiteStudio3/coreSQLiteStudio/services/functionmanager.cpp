#include "services/functionmanager.h"

FunctionManager::FunctionBase::FunctionBase()
{
}

FunctionManager::FunctionBase::~FunctionBase()
{
}

QString FunctionManager::FunctionBase::toString() const
{
    static const QString format = "%1(%2)";
    QString args = undefinedArgs ? "..." : arguments.join(", ");
    return format.arg(name).arg(args);
}

QString FunctionManager::FunctionBase::typeString(Type type)
{
    switch (type)
    {
        case ScriptFunction::SCALAR:
            return "SCALAR";
        case ScriptFunction::AGGREGATE:
            return "AGGREGATE";
        case ScriptFunction::AGG_WINDOW:
            return "AGG_WINDOW";
    }
    return QString();
}

FunctionManager::ScriptFunction::Type FunctionManager::FunctionBase::typeString(const QString& type)
{
    if (type == "SCALAR")
        return ScriptFunction::SCALAR;

    if (type == "AGGREGATE")
        return ScriptFunction::AGGREGATE;

    if (type == "AGG_WINDOW")
        return ScriptFunction::AGG_WINDOW;

    return ScriptFunction::SCALAR;
}
