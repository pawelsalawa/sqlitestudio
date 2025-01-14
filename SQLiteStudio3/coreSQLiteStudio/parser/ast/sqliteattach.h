#ifndef SQLITEATTACHDATABASE_H
#define SQLITEATTACHDATABASE_H

#include "sqlitequery.h"

class SqliteExpr;

class API_EXPORT SqliteAttach : public SqliteQuery
{
    Q_OBJECT

    public:
        SqliteAttach();
        SqliteAttach(const SqliteAttach& other);
        SqliteAttach(bool dbKw, SqliteExpr* url, SqliteExpr* name, SqliteExpr* key);
        ~SqliteAttach();
        SqliteStatement* clone();

        bool databaseKw = false;
        SqliteExpr* databaseUrl = nullptr;
        SqliteExpr* name = nullptr;
        SqliteExpr* key = nullptr;

    protected:
        TokenList rebuildTokensFromContents();
};

typedef QSharedPointer<SqliteAttach> SqliteAttachPtr;

#endif // SQLITEATTACHDATABASE_H
