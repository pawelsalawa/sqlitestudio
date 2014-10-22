#ifndef DBMANAGERMOCK_H
#define DBMANAGERMOCK_H

#include "services/dbmanager.h"

class DbManagerMock : public DbManager
{
    public:
        bool addDb(const QString& name, const QString&, const QHash<QString, QVariant>&, bool);
        bool addDb(const QString&, const QString&, bool);
        bool updateDb(Db*, const QString&, const QString&, const QHash<QString, QVariant>&, bool);
        void removeDbByName(const QString&, Qt::CaseSensitivity);
        void removeDbByPath(const QString&);
        void removeDb(Db*);
        QList<Db*> getDbList();
        QList<Db*> getValidDbList();
        QList<Db*> getConnectedDbList();
        QStringList getDbNames();
        Db*getByName(const QString&, Qt::CaseSensitivity);
        Db*getByPath(const QString&);
        Db*createInMemDb();
        bool isTemporary(Db*);

    public slots:
        void notifyDatabasesAreLoaded();
};

#endif // DBMANAGERMOCK_H
