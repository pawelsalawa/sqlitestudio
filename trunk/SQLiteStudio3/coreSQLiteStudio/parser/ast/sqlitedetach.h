#ifndef SQLITEDETACH_H
#define SQLITEDETACH_H

#include "sqlitequery.h"

class SqliteExpr;

class API_EXPORT SqliteDetach : public SqliteQuery
{
    public:
        SqliteDetach();
        SqliteDetach(bool databaseKw, SqliteExpr* name);
        ~SqliteDetach();

        bool databaseKw = false;
        SqliteExpr* name = nullptr;
};

typedef QSharedPointer<SqliteDetach> SqliteDetachPtr;

#endif // SQLITEDETACH_H
