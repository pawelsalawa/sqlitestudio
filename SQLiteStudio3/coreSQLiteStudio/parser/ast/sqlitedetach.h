#ifndef SQLITEDETACH_H
#define SQLITEDETACH_H

#include "sqlitequery.h"

class SqliteExpr;

class API_EXPORT SqliteDetach : public SqliteQuery
{
    Q_OBJECT

    public:
        SqliteDetach();
        SqliteDetach(const SqliteDetach& other);
        SqliteDetach(bool databaseKw, SqliteExpr* name);
        ~SqliteDetach();

        SqliteStatement* clone();

        bool databaseKw = false;
        SqliteExpr* name = nullptr;

    protected:
        TokenList rebuildTokensFromContents();
};

typedef QSharedPointer<SqliteDetach> SqliteDetachPtr;

#endif // SQLITEDETACH_H
