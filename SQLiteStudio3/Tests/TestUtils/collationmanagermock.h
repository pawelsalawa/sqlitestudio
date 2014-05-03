#ifndef COLLATIONMANAGERMOCK_H
#define COLLATIONMANAGERMOCK_H

#include "services/collationmanager.h"

class CollationManagerMock : public CollationManager
{
    public:
        CollationManagerMock();

        void setCollations(const QList<CollationPtr>& newCollations);
        QList<CollationPtr> getAllCollations() const;
        QList<CollationPtr> getCollationsForDatabase(const QString& dbName) const;
        int evaluate(const QString& name, const QString& value1, const QString& value2);
        int evaluateDefault(const QString& value1, const QString& value2);
};

#endif // COLLATIONMANAGERMOCK_H
