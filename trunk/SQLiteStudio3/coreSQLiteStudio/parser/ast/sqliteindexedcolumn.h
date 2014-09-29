#ifndef SQLITEINDEXEDCOLUMN_H
#define SQLITEINDEXEDCOLUMN_H

#include "sqlitestatement.h"
#include "sqlitesortorder.h"
#include <QString>

class API_EXPORT SqliteIndexedColumn : public SqliteStatement
{
    public:
        SqliteIndexedColumn();
        SqliteIndexedColumn(const SqliteIndexedColumn& other);
        SqliteIndexedColumn(const QString& name, const QString& collate, SqliteSortOrder sortOrder);
        explicit SqliteIndexedColumn(const QString& name);

        SqliteStatement* clone();

        QString name = QString::null;
        SqliteSortOrder sortOrder = SqliteSortOrder::null;
        QString collate = QString::null;

    protected:
        QStringList getColumnsInStatement();
        TokenList getColumnTokensInStatement();
        TokenList rebuildTokensFromContents();
};

typedef QSharedPointer<SqliteIndexedColumn> SqliteIndexedColumnPtr;

#endif // SQLITEINDEXEDCOLUMN_H
