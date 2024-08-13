#include "collationmanagermock.h"

CollationManagerMock::CollationManagerMock()
{
}

void CollationManagerMock::setCollations(const QList<CollationManager::CollationPtr>&)
{
}

QList<CollationManager::CollationPtr> CollationManagerMock::getAllCollations() const
{
    return QList<CollationManager::CollationPtr>();
}

CollationManager::CollationPtr CollationManagerMock::getCollation(const QString&) const
{
   return nullptr;
}

QList<CollationManager::CollationPtr> CollationManagerMock::getCollationsForDatabase(const QString&) const
{
    return QList<CollationManager::CollationPtr>();
}

int CollationManagerMock::evaluate(const QString&, const QString&, const QString&)
{
    return 0;
}

int CollationManagerMock::evaluateDefault(const QString&, const QString&)
{
    return 0;
}
