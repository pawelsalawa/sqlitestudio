#ifndef SQLITEWITH_H
#define SQLITEWITH_H

#include "sqlitestatement.h"
#include "sqliteindexedcolumn.h"

class SqliteSelect;

class SqliteWith : public SqliteStatement
{
    public:
        class CommonTableExpression : public SqliteStatement
        {
            public:
                CommonTableExpression(const QString& tableName, const QList<SqliteIndexedColumn*>& indexedColumns, SqliteSelect* select);

                QString table;
                QList<SqliteIndexedColumn*> indexedColumns;
                SqliteSelect* select = nullptr;

            protected:
                TokenList rebuildTokensFromContents();
        };

        SqliteWith();
        static SqliteWith* append(const QString& tableName, const QList<SqliteIndexedColumn*>& indexedColumns, SqliteSelect* select);
        static SqliteWith* append(SqliteWith* with, const QString& tableName, const QList<SqliteIndexedColumn*>& indexedColumns, SqliteSelect* select);

        QList<CommonTableExpression*> cteList;
        bool recursive = false;

    protected:
        TokenList rebuildTokensFromContents();
};

typedef QSharedPointer<SqliteWith> SqliteWithPtr;

#endif // SQLITEWITH_H
