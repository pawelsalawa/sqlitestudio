#ifndef SQLITEUPSERT_H
#define SQLITEUPSERT_H


#include "sqlitestatement.h"
#include "sqliteindexedcolumn.h"

class SqliteExpr;
class SqliteOrderBy;

class SqliteUpsert : public SqliteStatement
{
    Q_OBJECT

    public:
        typedef QPair<QVariant,SqliteExpr*> ColumnAndValue;

        SqliteUpsert();
        SqliteUpsert(const QList<SqliteOrderBy*>& conflictColumns, SqliteExpr* conflictWhere);
        SqliteUpsert(const QList<SqliteOrderBy*>& conflictColumns, SqliteExpr* conflictWhere, const QList<ColumnAndValue>& values, SqliteExpr* setWhere);
        SqliteUpsert(const SqliteUpsert& other);

        SqliteStatement* clone();

        QList<SqliteOrderBy*> conflictColumns;
        SqliteExpr* conflictWhere = nullptr;
        QList<ColumnAndValue> keyValueMap;
        SqliteExpr* setWhere = nullptr;
        bool doNothing = false;

    protected:
        TokenList rebuildTokensFromContents();
        QStringList getColumnsInStatement();
        TokenList getColumnTokensInStatement();
};

#endif // SQLITEUPSERT_H
