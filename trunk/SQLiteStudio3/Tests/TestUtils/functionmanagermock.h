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
        void evaluateAggregateInitial(const QString&, int, Db*, QHash<QString, QVariant>&);
        void evaluateAggregateStep(const QString&, int, const QList<QVariant>&, Db*, QHash<QString, QVariant>&);
        QVariant evaluateAggregateFinal(const QString&, int, Db*, bool&, QHash<QString, QVariant>&);
};

#endif // FUNCTIONMANAGERMOCK_H
