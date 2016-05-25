#ifndef SQLITEORDERBY_H
#define SQLITEORDERBY_H

#include "sqlitestatement.h"
#include "sqlitesortorder.h"
#include "sqliteextendedindexedcolumn.h"

class SqliteExpr;

class API_EXPORT SqliteOrderBy : public SqliteStatement, public SqliteExtendedIndexedColumn
{
    public:
        SqliteOrderBy();
        SqliteOrderBy(const SqliteOrderBy& other);
        SqliteOrderBy(SqliteExpr* expr, SqliteSortOrder order);
        ~SqliteOrderBy();

        SqliteStatement* clone();
        bool isSimpleColumn() const;
        QString getColumnName() const;
        QString getCollation() const;
        QString getColumnString() const;
        void setColumnName(const QString& name);
        void setCollation(const QString& name);
        void clearCollation();

        SqliteExpr* expr = nullptr;
        SqliteSortOrder order = SqliteSortOrder::null;

    protected:
        TokenList rebuildTokensFromContents();
};

typedef QSharedPointer<SqliteOrderBy> SqliteOrderByPtr;

#endif // SQLITEORDERBY_H
