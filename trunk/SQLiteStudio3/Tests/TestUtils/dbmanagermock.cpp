#include "dbmanagermock.h"

bool DbManagerMock::addDb(const QString& name, const QString& path, const QHash<QString, QVariant>& options, bool permanent)
{
    return true;
}

bool DbManagerMock::addDb(const QString& name, const QString& path, bool permanent)
{
    return true;
}

bool DbManagerMock::updateDb(Db* db, const QString& name, const QString& path, const QHash<QString, QVariant>& options, bool permanent)
{
    return true;
}

void DbManagerMock::removeDbByName(const QString& name, Qt::CaseSensitivity cs)
{
}

void DbManagerMock::removeDbByPath(const QString& path)
{
}

void DbManagerMock::removeDb(Db* db)
{
}

QList<Db*> DbManagerMock::getDbList()
{
    return QList<Db*>();
}

QList<Db*> DbManagerMock::getConnectedDbList()
{
    return QList<Db*>();
}

QStringList DbManagerMock::getDbNames()
{
    return QStringList();
}

Db* DbManagerMock::getByName(const QString& name, Qt::CaseSensitivity cs)
{
    return nullptr;
}

Db* DbManagerMock::getByPath(const QString& path)
{
    return nullptr;
}

void DbManagerMock::loadDbListFromConfig()
{
}
