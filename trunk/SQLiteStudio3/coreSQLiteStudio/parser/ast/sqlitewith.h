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
                CommonTableExpression();
                CommonTableExpression(const CommonTableExpression& other);
                CommonTableExpression(const QString& tableName, const QList<SqliteIndexedColumn*>& indexedColumns, SqliteSelect* select);

                SqliteStatement* clone();

                QString table;
                QList<SqliteIndexedColumn*> indexedColumns;
                SqliteSelect* select = nullptr;

            protected:
                TokenList rebuildTokensFromContents();
        };

        SqliteWith();
        SqliteWith(const SqliteWith& other);
        static SqliteWith* append(const QString& tableName, const QList<SqliteIndexedColumn*>& indexedColumns, SqliteSelect* select);
        static SqliteWith* append(SqliteWith* with, const QString& tableName, const QList<SqliteIndexedColumn*>& indexedColumns, SqliteSelect* select);

        SqliteStatement* clone();

        QList<CommonTableExpression*> cteList;
        bool recursive = false;

    protected:
        TokenList rebuildTokensFromContents();
};

typedef QSharedPointer<SqliteWith> SqliteWithPtr;

#endif // SQLITEWITH_H
