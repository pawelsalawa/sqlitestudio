#ifndef SQLITEREINDEX_H
#define SQLITEREINDEX_H

#include "sqlitequery.h"

#include <QString>

class API_EXPORT SqliteReindex : public SqliteQuery
{
    public:
        SqliteReindex();
        SqliteReindex(const QString& name1, const QString& name2);

        QString database = QString::null;
        QString table = QString::null;

    protected:
        QStringList getTablesInStatement();
        QStringList getDatabasesInStatement();
        TokenList getTableTokensInStatement();
        TokenList getDatabaseTokensInStatement();
        QList<SqliteStatement::FullObject> getFullObjectsInStatement();
        TokenList rebuildTokensFromContents();
};

typedef QSharedPointer<SqliteReindex> SqliteReindexPtr;

#endif // SQLITEREINDEX_H
