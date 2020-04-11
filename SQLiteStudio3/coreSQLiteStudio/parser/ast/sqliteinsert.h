#ifndef SQLITEINSERT_H
#define SQLITEINSERT_H

#include "sqlitequery.h"
#include "sqliteconflictalgo.h"
#include <QString>
#include <QList>

class SqliteSelect;
class SqliteExpr;
class SqliteWith;
class SqliteUpsert;

class API_EXPORT SqliteInsert : public SqliteQuery
{
    public:
        SqliteInsert();
        SqliteInsert(const SqliteInsert& other);
        SqliteInsert(bool replace, SqliteConflictAlgo onConflict, const QString& name1,
                     const QString& name2, const QList<QString>& columns,
                     const QList<SqliteExpr*>& row, SqliteWith* with);
        SqliteInsert(bool replace, SqliteConflictAlgo onConflict, const QString& name1,
                     const QString& name2, const QList<QString>& columns, SqliteSelect* select, SqliteWith* with, SqliteUpsert* upsert = nullptr);
        SqliteInsert(bool replace, SqliteConflictAlgo onConflict, const QString& name1,
                     const QString& name2, const QList<QString>& columns, SqliteWith* with);
        ~SqliteInsert();

        SqliteStatement* clone();

    protected:
        QStringList getColumnsInStatement();
        QStringList getTablesInStatement();
        QStringList getDatabasesInStatement();
        TokenList getColumnTokensInStatement();
        TokenList getTableTokensInStatement();
        TokenList getDatabaseTokensInStatement();
        QList<FullObject> getFullObjectsInStatement();
        TokenList rebuildTokensFromContents();

    private:
        void initName(const QString& name1, const QString& name2);
        void initMode(bool replace, SqliteConflictAlgo onConflict);

    public:
        bool replaceKw = false;
        bool defaultValuesKw = false;
        SqliteConflictAlgo onConflict = SqliteConflictAlgo::null;
        QString database = QString();
        QString table = QString();
        QStringList columnNames;
        QList<SqliteExpr*> values;
        SqliteSelect* select = nullptr;
        SqliteWith* with = nullptr;
        SqliteUpsert* upsert = nullptr;
};

typedef QSharedPointer<SqliteInsert> SqliteInsertPtr;

#endif // SQLITEINSERT_H
