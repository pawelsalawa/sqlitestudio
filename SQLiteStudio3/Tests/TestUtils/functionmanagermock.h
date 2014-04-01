#ifndef FUNCTIONMANAGERMOCK_H
#define FUNCTIONMANAGERMOCK_H

#include "services/functionmanager.h"

#include <QVariant>

class FunctionManagerMock : public FunctionManager
{
    public:
        void setFunctions(const QList<FunctionPtr>&);
        QList<FunctionPtr> getAllFunctions() const;
        QList<FunctionPtr> getFunctionsForDatabase(const QString&) const;
        QVariant evaluateScalar(const QString&, int, const QList<QVariant>&, Db*, bool&);
        void evaluateAggregateInitial(const QString&, int, Db*, QHash<QString, QVariant>&);
        void evaluateAggregateStep(const QString&, int, const QList<QVariant>&, Db*, QHash<QString, QVariant>&);
        QVariant evaluateAggregateFinal(const QString&, int, Db*, bool&, QHash<QString, QVariant>&);
};

#endif // FUNCTIONMANAGERMOCK_H
