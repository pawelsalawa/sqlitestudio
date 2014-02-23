#ifndef DBSQLITE3MOCK_H
#define DBSQLITE3MOCK_H

#include "db/dbqt.h"

class DbSqlite3Mock : public DbQt
{
    Q_OBJECT
    public:
        DbSqlite3Mock(const QString& name, const QString& path = ":memory:",
                      const QHash<QString, QVariant> &options = QHash<QString,QVariant>());

    protected:
        QString getDriver();
        void interruptExecutionOnHandle(const QVariant& handle);
};

#endif // DBSQLITE3MOCK_H
