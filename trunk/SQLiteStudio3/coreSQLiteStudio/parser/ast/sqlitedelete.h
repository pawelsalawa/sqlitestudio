#ifndef SQLITEDELETE_H
#define SQLITEDELETE_H

#include "sqlitequery.h"

#include <QString>

class SqliteExpr;
class SqliteWith;

class API_EXPORT SqliteDelete : public SqliteQuery
{
    public:
        SqliteDelete();
        SqliteDelete(const SqliteDelete& other);
        SqliteDelete(const QString& name1, const QString& name2, const QString& indexedByName, SqliteExpr* where, SqliteWith* with);
        SqliteDelete(const QString& name1, const QString& name2, bool notIndexedKw, SqliteExpr* where, SqliteWith* with);
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
        void init(const QString& name1, const QString& name2, SqliteExpr* where, SqliteWith* with);

    public:
        QString database = QString::null;
        QString table = QString::null;
        bool indexedByKw = false;
        bool notIndexedKw = false;
        QString indexedBy = QString::null;
        SqliteExpr* where = nullptr;
        SqliteWith* with = nullptr;
};

typedef QSharedPointer<SqliteDelete> SqliteDeletePtr;

#endif // SQLITEDELETE_H
