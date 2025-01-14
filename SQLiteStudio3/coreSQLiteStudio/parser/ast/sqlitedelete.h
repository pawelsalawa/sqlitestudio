#ifndef SQLITEDELETE_H
#define SQLITEDELETE_H

#include "sqlitequery.h"
#include "sqlitequerywithaliasedtable.h"
#include "sqliteselect.h"
#include <QString>
#include <QList>

class SqliteExpr;
class SqliteWith;

class API_EXPORT SqliteDelete : public SqliteQuery, SqliteQueryWithAliasedTable
{
    Q_OBJECT

    public:
        SqliteDelete();
        SqliteDelete(const SqliteDelete& other);
        SqliteDelete(const QString& name1, const QString& name2, const QString& alias, const QString& indexedByName, SqliteExpr* where, SqliteWith* with,
                     const QList<SqliteResultColumn*>& returning, const QList<SqliteOrderBy*>& orderBy, SqliteLimit* limit);
        SqliteDelete(const QString& name1, const QString& name2, const QString& alias, bool notIndexedKw, SqliteExpr* where, SqliteWith* with,
                     const QList<SqliteResultColumn*>& returning, const QList<SqliteOrderBy*>& orderBy, SqliteLimit* limit);
        ~SqliteDelete();

        SqliteStatement* clone();
        QString getTable() const;
        QString getDatabase() const;
        QString getTableAlias() const;

    protected:
        QStringList getTablesInStatement();
        QStringList getDatabasesInStatement();
        TokenList getTableTokensInStatement();
        TokenList getDatabaseTokensInStatement();
        QList<FullObject> getFullObjectsInStatement();
        TokenList rebuildTokensFromContents();

    private:
        void init(const QString& name1, const QString& name2, const QString& alias, SqliteExpr* where, SqliteWith* with,
                  const QList<SqliteResultColumn*>& returning, const QList<SqliteOrderBy*>& orderBy, SqliteLimit* limit);

    public:
        QString database = QString();
        QString table = QString();
        QString tableAlias = QString();
        bool indexedByKw = false;
        bool notIndexedKw = false;
        QString indexedBy = QString();
        SqliteExpr* where = nullptr;
        SqliteWith* with = nullptr;
        QList<SqliteResultColumn*> returning;
        QList<SqliteOrderBy*> orderBy;
        SqliteLimit* limit = nullptr;
};

typedef QSharedPointer<SqliteDelete> SqliteDeletePtr;

#endif // SQLITEDELETE_H
