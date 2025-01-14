#ifndef SQLITEINSERT_H
#define SQLITEINSERT_H

#include "sqlitequery.h"
#include "sqliteconflictalgo.h"
#include "sqliteselect.h"
#include "sqlitequerywithaliasedtable.h"
#include <QString>
#include <QList>

class SqliteExpr;
class SqliteWith;
class SqliteUpsert;

class API_EXPORT SqliteInsert : public SqliteQuery, SqliteQueryWithAliasedTable
{
    Q_OBJECT

    public:
        SqliteInsert();
        SqliteInsert(const SqliteInsert& other);
        SqliteInsert(bool replace, SqliteConflictAlgo onConflict, const QString& name1,
                     const QString& name2, const QString& alias, const QList<QString>& columns,
                     const QList<SqliteExpr*>& row, SqliteWith* with,
                     const QList<SqliteResultColumn*>& returning);
        SqliteInsert(bool replace, SqliteConflictAlgo onConflict, const QString& name1,
                     const QString& name2, const QString& alias, const QList<QString>& columns,
                     SqliteSelect* select, SqliteWith* with,
                     SqliteUpsert* upsert, const QList<SqliteResultColumn*>& returning);
        SqliteInsert(bool replace, SqliteConflictAlgo onConflict, const QString& name1,
                     const QString& name2, const QString& alias, const QList<QString>& columns,
                     SqliteWith* with, const QList<SqliteResultColumn*>& returning);
        ~SqliteInsert();

        SqliteStatement* clone();
        QString getTable() const;
        QString getDatabase() const;
        QString getTableAlias() const;

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
        void init(const QString& name1, const QString& name2, const QString& alias, bool replace,
                  SqliteConflictAlgo onConflict, const QList<SqliteResultColumn*>& returning);

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
        QList<SqliteResultColumn*> returning;
        QString tableAlias = QString();
};

typedef QSharedPointer<SqliteInsert> SqliteInsertPtr;

#endif // SQLITEINSERT_H
