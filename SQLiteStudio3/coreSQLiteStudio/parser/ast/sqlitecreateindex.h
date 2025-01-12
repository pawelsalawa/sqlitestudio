#ifndef SQLITECREATEINDEX_H
#define SQLITECREATEINDEX_H

#include "sqlitequery.h"
#include "sqlitetablerelatedddl.h"
#include "sqliteconflictalgo.h"
#include "sqliteexpr.h"
#include "sqliteddlwithdbcontext.h"
#include "sqliteorderby.h"
#include <QString>
#include <QList>

class SqliteIndexedColumn;

class API_EXPORT SqliteCreateIndex : public SqliteQuery, public SqliteTableRelatedDdl, public SqliteDdlWithDbContext
{
    Q_OBJECT

    public:
        SqliteCreateIndex();
        SqliteCreateIndex(const SqliteCreateIndex& other);
        SqliteCreateIndex(bool unique, bool ifNotExists, const QString& name1, const QString& name2,
                          const QString& name3, const QList<SqliteIndexedColumn*>& columns,
                          SqliteConflictAlgo onConflict = SqliteConflictAlgo::null);
        SqliteCreateIndex(bool unique, bool ifNotExists, const QString& name1, const QString& name2,
                          const QString& name3, const QList<SqliteOrderBy*>& columns,
                          SqliteExpr* where);
        ~SqliteCreateIndex();
        SqliteStatement* clone();

        QString getTargetTable() const;
        QString getTargetDatabase() const;
        void setTargetDatabase(const QString& database);
        QString getObjectName() const;
        void setObjectName(const QString& name);

        bool uniqueKw = false;
        bool ifNotExistsKw = false;
        QList<SqliteOrderBy*> indexedColumns;
        QString database = QString();
        QString index = QString();
        QString table = QString();
        SqliteConflictAlgo onConflict = SqliteConflictAlgo::null;
        SqliteExpr* where = nullptr;

    protected:
        QStringList getTablesInStatement();
        QStringList getDatabasesInStatement();
        TokenList getTableTokensInStatement();
        TokenList getDatabaseTokensInStatement();
        QList<FullObject> getFullObjectsInStatement();
        TokenList rebuildTokensFromContents();

    private:
        QList<SqliteOrderBy*> toOrderColumns(const QList<SqliteIndexedColumn*>& columns);
};

typedef QSharedPointer<SqliteCreateIndex> SqliteCreateIndexPtr;

#endif // SQLITECREATEINDEX_H
