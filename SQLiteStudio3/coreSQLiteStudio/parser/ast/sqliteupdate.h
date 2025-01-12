#ifndef SQLITEUPDATE_H
#define SQLITEUPDATE_H

#include "sqlitequery.h"
#include "sqliteconflictalgo.h"
#include "sqliteselect.h"
#include "sqlitequerywithaliasedtable.h"

#include <QStringList>
#include <QMap>

class SqliteExpr;
class SqliteWith;

class API_EXPORT SqliteUpdate : public SqliteQuery, SqliteQueryWithAliasedTable
{
    Q_OBJECT

    public:
        typedef QPair<QVariant,SqliteExpr*> ColumnAndValue;

        SqliteUpdate();
        SqliteUpdate(const SqliteUpdate& other);
        ~SqliteUpdate();
        SqliteUpdate(SqliteConflictAlgo onConflict, const QString& name1, const QString& name2, const QString& alias,
                     bool notIndexedKw, const QString& indexedBy, const QList<ColumnAndValue>& values,
                     SqliteSelect::Core::JoinSource* from, SqliteExpr* where, SqliteWith* with, const QList<SqliteResultColumn*>& returning,
                     const QList<SqliteOrderBy*>& orderBy, SqliteLimit* limit);

        SqliteStatement* clone();
        SqliteExpr* getValueForColumnSet(const QString& column);
        QString getTable() const;
        QString getDatabase() const;
        QString getTableAlias() const;

        SqliteConflictAlgo onConflict = SqliteConflictAlgo::null;
        QString database = QString();
        QString table = QString();
        QString tableAlias = QString();
        bool indexedByKw = false;
        bool notIndexedKw = false;
        QString indexedBy = QString();
        QList<ColumnAndValue> keyValueMap;
        SqliteSelect::Core::JoinSource* from = nullptr;
        SqliteExpr* where = nullptr;
        SqliteWith* with = nullptr;
        QList<SqliteResultColumn*> returning;
        QList<SqliteOrderBy*> orderBy;
        SqliteLimit* limit = nullptr;

    protected:
        QStringList getColumnsInStatement();
        QStringList getTablesInStatement();
        QStringList getDatabasesInStatement();
        TokenList getColumnTokensInStatement();
        TokenList getTableTokensInStatement();
        TokenList getDatabaseTokensInStatement();
        QList<FullObject> getFullObjectsInStatement();
        TokenList rebuildTokensFromContents();
};

typedef QSharedPointer<SqliteUpdate> SqliteUpdatePtr;

#endif // SQLITEUPDATE_H
