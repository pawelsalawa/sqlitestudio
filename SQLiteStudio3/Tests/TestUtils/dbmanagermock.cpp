#include "dbmanagermock.h"

bool DbManagerMock::addDb(const QString&, const QString&, const QHash<QString, QVariant>&, bool)
{
    return true;
}

bool DbManagerMock::addDb(const QString&, const QString&, bool)
{
    return true;
}

bool DbManagerMock::updateDb(Db*, const QString&, const QString&, const QHash<QString, QVariant>&, bool)
{
    return true;
}

void DbManagerMock::removeDbByName(const QString&, Qt::CaseSensitivity)
{
}

void DbManagerMock::removeDbByPath(const QString&)
{
}

void DbManagerMock::removeDb(Db*)
{
}

QList<Db*> DbManagerMock::getDbList()
{
    return QList<Db*>();
}

QList<Db*> DbManagerMock::getValidDbList()
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

QStringList DbManagerMock::getValidDbNames()
{
    return QStringList();
}

Db* DbManagerMock::getByName(const QString&, Qt::CaseSensitivity)
{
    return nullptr;
}

Db* DbManagerMock::getByPath(const QString&)
{
    return nullptr;
}

Db*DbManagerMock::createInMemDb(bool)
{
    return nullptr;
}

bool DbManagerMock::isTemporary(Db*)
{
    return false;
}

DbPlugin* DbManagerMock::getPluginForDbFile(const QString&)
{
    return nullptr;
}

QString DbManagerMock::generateUniqueDbName(const QString&)
{
    return QString();
}

QString DbManagerMock::generateUniqueDbName(DbPlugin*, const QString&)
{
    return QString();
}

QString DbManagerMock::quickAddDb(const QString &, const QHash<QString, QVariant> &)
{
    return QString();
}

void DbManagerMock::notifyDatabasesAreLoaded()
{
}

void DbManagerMock::scanForNewDatabasesInConfig()
{
}

void DbManagerMock::rescanInvalidDatabasesForPlugin(DbPlugin*)
{
}
