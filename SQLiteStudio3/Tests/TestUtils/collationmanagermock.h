#ifndef COLLATIONMANAGERMOCK_H
#define COLLATIONMANAGERMOCK_H

#include "services/collationmanager.h"

class CollationManagerMock : public CollationManager
{
    public:
        CollationManagerMock();

        void setCollations(const QList<CollationPtr>&);
        QList<CollationPtr> getAllCollations() const;
        QList<CollationPtr> getCollationsForDatabase(const QString&) const;
        int evaluate(const QString&, const QString&, const QString&);
        int evaluateDefault(const QString&, const QString&);
        CollationPtr getCollation(const QString &name) const;
};

#endif // COLLATIONMANAGERMOCK_H
