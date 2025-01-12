#ifndef SQLITEWITH_H
#define SQLITEWITH_H

#include "sqlitestatement.h"
#include "sqliteindexedcolumn.h"

class SqliteSelect;

class API_EXPORT SqliteWith : public SqliteStatement
{
    Q_OBJECT

    public:
        class API_EXPORT CommonTableExpression : public SqliteStatement
        {
            public:
                enum AsMode {
                    ANY,
                    MATERIALIZED,
                    NOT_MATERIALIZED
                };

                CommonTableExpression();
                CommonTableExpression(const CommonTableExpression& other);
                CommonTableExpression(const QString& tableName, const QList<SqliteIndexedColumn*>& indexedColumns, SqliteSelect* select,
                                      AsMode asMode);

                SqliteStatement* clone();

                QString table;
                QList<SqliteIndexedColumn*> indexedColumns;
                SqliteSelect* select = nullptr;
                AsMode asMode = ANY;

            protected:
                TokenList rebuildTokensFromContents();
        };

        SqliteWith();
        SqliteWith(const SqliteWith& other);

        SqliteStatement* clone();

        QList<CommonTableExpression*> cteList;
        bool recursive = false;

    protected:
        TokenList rebuildTokensFromContents();
};

typedef QSharedPointer<SqliteWith> SqliteWithPtr;

#endif // SQLITEWITH_H
