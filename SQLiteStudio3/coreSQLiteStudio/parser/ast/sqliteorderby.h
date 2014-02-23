#ifndef SQLITEORDERBY_H
#define SQLITEORDERBY_H

#include "sqlitestatement.h"
#include "sqlitesortorder.h"

class SqliteExpr;

class API_EXPORT SqliteOrderBy : public SqliteStatement
{
    public:
        SqliteOrderBy();
        SqliteOrderBy(const SqliteOrderBy& other);
        SqliteOrderBy(SqliteExpr* expr, SqliteSortOrder order);
        ~SqliteOrderBy();

        SqliteExpr* expr = nullptr;
        SqliteSortOrder order;

    protected:
        TokenList rebuildTokensFromContents();
};

typedef QSharedPointer<SqliteOrderBy> SqliteOrderByPtr;

#endif // SQLITEORDERBY_H
