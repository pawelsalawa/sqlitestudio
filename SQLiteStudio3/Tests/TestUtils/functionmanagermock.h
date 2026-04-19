#ifndef FUNCTIONMANAGERMOCK_H
#define FUNCTIONMANAGERMOCK_H

#include "services/functionmanager.h"

#include <QVariant>

class FunctionManagerMock : public FunctionManager
{
    public:
        void setScriptFunctions(const QList<ScriptFunction*>&);
        QList<ScriptFunction*> getAllScriptFunctions() const;
        QList<ScriptFunction*> getScriptFunctionsForDatabase(const QString&) const;
        QList<NativeFunction*> getAllNativeFunctions() const;
        QVariant evaluateScalar(const QString&, int, const QList<QVariant>&, Db*, bool&);
        QVariant evaluateWindowValue(const QString&, int, Db*, bool&, QHash<QString, QVariant>&);
        void evaluateWindowInverse(const QString&, int, const QList<QVariant>&, Db*, QHash<QString, QVariant>&);
        void evaluateAggregateInitial(const QString& name, int argCount, Db* db, QHash<QString, QVariant>& aggregateStorage, FunctionBase::Type);
        void evaluateAggregateStep(const QString& name, int argCount, const QList<QVariant>& args, Db* db, QHash<QString, QVariant>& aggregateStorage, FunctionBase::Type);
        QVariant evaluateAggregateFinal(const QString& name, int argCount, Db* db, bool& ok, QHash<QString, QVariant>& aggregateStorage, FunctionBase::Type);
        void loadFromConfig();
        void init();
};

#endif // FUNCTIONMANAGERMOCK_H
