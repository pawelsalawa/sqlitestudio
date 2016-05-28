#ifndef DBATTACHERMOCK_H
#define DBATTACHERMOCK_H

#include "dbattacher.h"

class DbAttacherMock : public DbAttacher
{
    public:
        bool attachDatabases(const QString&);
        bool attachDatabases(const QList<SqliteQueryPtr>&);
        bool attachDatabases(SqliteQueryPtr);
        void detachDatabases();
        BiStrHash getDbNameToAttach() const;
        QString getQuery() const;
        bool getMainDbNameUsed() const;
};

class DbAttacherFactoryMock : public DbAttacherFactory
{
    public:
        DbAttacher* create(Db*);
};

#endif // DBATTACHERMOCK_H
