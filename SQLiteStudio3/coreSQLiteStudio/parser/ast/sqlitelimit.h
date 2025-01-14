#ifndef SQLITELIMIT_H
#define SQLITELIMIT_H

#include "sqlitestatement.h"

class SqliteExpr;

class API_EXPORT SqliteLimit : public SqliteStatement
{
    Q_OBJECT

    public:
        SqliteLimit();
        SqliteLimit(const SqliteLimit& other);
        explicit SqliteLimit(SqliteExpr* expr);
        SqliteLimit(SqliteExpr* expr1, SqliteExpr* expr2, bool offsetKeyword);
        explicit SqliteLimit(const QVariant& positiveInt);
        SqliteLimit(const QVariant& positiveInt1, const QVariant& positiveInt2);
        ~SqliteLimit();

        SqliteStatement* clone();

        SqliteExpr* limit = nullptr;
        SqliteExpr* offset = nullptr;
        bool offsetKw = false;

    protected:
        TokenList rebuildTokensFromContents();
};

typedef QSharedPointer<SqliteLimit> SqliteLimitPtr;

#endif // SQLITELIMIT_H
