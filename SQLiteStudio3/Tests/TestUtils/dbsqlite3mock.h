#ifndef DBSQLITE3MOCK_H
#define DBSQLITE3MOCK_H

#include "db/dbqt3.h"

class Sqlite3Mock;

class DbSqlite3Mock : public DbQt3<Sqlite3Mock>
{
    Q_OBJECT
    public:
        DbSqlite3Mock(const QString& name, const QString& path = ":memory:",
                      const QHash<QString, QVariant> &options = QHash<QString,QVariant>());

    protected:
        QString getDriver();
        void interruptExecutionOnHandle(const QVariant& handle);

    public slots:
        void registerAllFunctions();
};

#endif // DBSQLITE3MOCK_H
