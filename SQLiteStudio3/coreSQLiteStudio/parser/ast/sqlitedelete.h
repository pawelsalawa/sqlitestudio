#ifndef SQLITEDELETE_H
#define SQLITEDELETE_H

#include "sqlitequery.h"
#include "sqliteselect.h"
#include <QString>
#include <QList>

class SqliteExpr;
class SqliteWith;

class API_EXPORT SqliteDelete : public SqliteQuery
{
    public:
        SqliteDelete();
        SqliteDelete(const SqliteDelete& other);
        SqliteDelete(const QString& name1, const QString& name2, const QString& indexedByName, SqliteExpr* where, SqliteWith* with,
                     const QList<SqliteResultColumn*>& returning);
        SqliteDelete(const QString& name1, const QString& name2, bool notIndexedKw, SqliteExpr* where, SqliteWith* with,
                     const QList<SqliteResultColumn*>& returning);
        ~SqliteDelete();

        SqliteStatement* clone();

    protected:
        QStringList getTablesInStatement();
        QStringList getDatabasesInStatement();
        TokenList getTableTokensInStatement();
        TokenList getDatabaseTokensInStatement();
        QList<FullObject> getFullObjectsInStatement();
        TokenList rebuildTokensFromContents();

    private:
        void init(const QString& name1, const QString& name2, SqliteExpr* where, SqliteWith* with,
                  const QList<SqliteResultColumn*>& returning);

    public:
        QString database = QString();
        QString table = QString();
        bool indexedByKw = false;
        bool notIndexedKw = false;
        QString indexedBy = QString();
        SqliteExpr* where = nullptr;
        SqliteWith* with = nullptr;
        QList<SqliteResultColumn*> returning;
};

typedef QSharedPointer<SqliteDelete> SqliteDeletePtr;

#endif // SQLITEDELETE_H
