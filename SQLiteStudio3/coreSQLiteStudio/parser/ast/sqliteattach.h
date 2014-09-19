#ifndef SQLITEATTACHDATABASE_H
#define SQLITEATTACHDATABASE_H

#include "sqlitequery.h"

class SqliteExpr;

class API_EXPORT SqliteAttach : public SqliteQuery
{
    public:
        SqliteAttach();
        SqliteAttach(bool dbKw, SqliteExpr* url, SqliteExpr* name, SqliteExpr* key);
        ~SqliteAttach();

        bool databaseKw = false;
        SqliteExpr* databaseUrl = nullptr;
        SqliteExpr* name = nullptr;
        SqliteExpr* key = nullptr;

    protected:
        TokenList rebuildTokensFromContents();
};

typedef QSharedPointer<SqliteAttach> SqliteAttachPtr;

#endif // SQLITEATTACHDATABASE_H
