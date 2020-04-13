#ifndef SQLITECOPY_H
#define SQLITECOPY_H

#include "sqlitequery.h"
#include "sqliteconflictalgo.h"

class API_EXPORT SqliteCopy : public SqliteQuery
{
    public:
        SqliteCopy();
        SqliteCopy(const SqliteCopy& other);
        SqliteCopy(SqliteConflictAlgo onConflict, const QString& name1, const QString& name2, const QString& name3, const QString& delim = QString());
        SqliteStatement* clone();

        SqliteConflictAlgo onConflict = SqliteConflictAlgo::null;
        QString database = QString();
        QString table = QString();
        QString file = QString();
        QString delimiter = QString();

    protected:
        QStringList getTablesInStatement();
        QStringList getDatabasesInStatement();
        TokenList getTableTokensInStatement();
        TokenList getDatabaseTokensInStatement();
        QList<FullObject> getFullObjectsInStatement();
        TokenList rebuildTokensFromContents();
};

typedef QSharedPointer<SqliteCopy> SqliteCopyPtr;

#endif // SQLITECOPY_H
