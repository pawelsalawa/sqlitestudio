#include "collationmanagermock.h"

CollationManagerMock::CollationManagerMock()
{
}

void CollationManagerMock::setCollations(const QList<CollationManager::CollationPtr>& newCollations)
{
}

QList<CollationManager::CollationPtr> CollationManagerMock::getAllCollations() const
{
    return QList<CollationManager::CollationPtr>();
}

QList<CollationManager::CollationPtr> CollationManagerMock::getCollationsForDatabase(const QString& dbName) const
{
    return QList<CollationManager::CollationPtr>();
}

int CollationManagerMock::evaluate(const QString& name, const QString& value1, const QString& value2)
{
    return 0;
}

int CollationManagerMock::evaluateDefault(const QString& value1, const QString& value2)
{
    return 0;
}
