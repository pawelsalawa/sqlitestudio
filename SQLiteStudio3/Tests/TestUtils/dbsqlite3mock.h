#ifndef DBSQLITE3MOCK_H
#define DBSQLITE3MOCK_H

#include "db/dbsqlite3.h"

class DbSqlite3Mock : public DbSqlite3
{
    Q_OBJECT

    public:
        DbSqlite3Mock(const QString& name, const QString& path = ":memory:",
                      const QHash<QString, QVariant> &options = QHash<QString,QVariant>());
};

#endif // DBSQLITE3MOCK_H
