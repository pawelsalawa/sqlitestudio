#ifndef DBATTACHERMOCK_H
#define DBATTACHERMOCK_H

#include "dbattacher.h"

class DbAttacherMock : public DbAttacher
{
    public:
        bool attachDatabases(const QString& query);
        bool attachDatabases(const QList<SqliteQueryPtr>& queries);
        bool attachDatabases(SqliteQueryPtr query);
        void detachDatabases();
        BiStrHash getDbNameToAttach() const;
        QString getQuery() const;
};

class DbAttacherFactoryMock : public DbAttacherFactory
{
    public:
        DbAttacher* create(Db* db);
};

#endif // DBATTACHERMOCK_H
