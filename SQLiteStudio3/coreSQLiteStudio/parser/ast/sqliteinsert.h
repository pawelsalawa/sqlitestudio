#ifndef SQLITEINSERT_H
#define SQLITEINSERT_H

#include "sqlitequery.h"
#include "sqliteconflictalgo.h"
#include <QString>
#include <QList>

class SqliteSelect;
class SqliteExpr;

class API_EXPORT SqliteInsert : public SqliteQuery
{
    public:
        SqliteInsert();
        SqliteInsert(const SqliteInsert& other);
        SqliteInsert(bool replace, SqliteConflictAlgo onConflict, const QString& name1,
                     const QString& name2, const QList<QString>& columns,
                     const QList<QList<SqliteExpr*> >& rows);
        SqliteInsert(bool replace, SqliteConflictAlgo onConflict, const QString& name1,
                     const QString& name2, const QList<QString>& columns,
                     const QList<SqliteExpr*>& row);
        SqliteInsert(bool replace, SqliteConflictAlgo onConflict, const QString& name1,
                     const QString& name2, const QList<QString>& columns, SqliteSelect* select);
        SqliteInsert(bool replace, SqliteConflictAlgo onConflict, const QString& name1,
                     const QString& name2, const QList<QString>& columns);
        ~SqliteInsert();

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
        QString database = QString::null;
        QString table = QString::null;
        QStringList columnNames;
        QList< QList<SqliteExpr*> > values;
        SqliteSelect* select;
};

typedef QSharedPointer<SqliteInsert> SqliteInsertPtr;

#endif // SQLITEINSERT_H
