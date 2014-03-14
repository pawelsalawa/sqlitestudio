#ifndef DBMANAGERMOCK_H
#define DBMANAGERMOCK_H

#include "services/dbmanager.h"

class DbManagerMock : public DbManager
{
    public:
        bool addDb(const QString& name, const QString& path, const QHash<QString, QVariant>& options, bool permanent);
        bool addDb(const QString& name, const QString& path, bool permanent);
        bool updateDb(Db* db, const QString& name, const QString& path, const QHash<QString, QVariant>& options, bool permanent);
        void removeDbByName(const QString& name, Qt::CaseSensitivity cs);
        void removeDbByPath(const QString& path);
        void removeDb(Db* db);
        QList<Db*> getDbList();
        QList<Db*> getConnectedDbList();
        QStringList getDbNames();
        Db*getByName(const QString& name, Qt::CaseSensitivity cs);
        Db*getByPath(const QString& path);

    public slots:
        void loadDbListFromConfig();
};

#endif // DBMANAGERMOCK_H
