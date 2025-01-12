#ifndef SQLITEREINDEX_H
#define SQLITEREINDEX_H

#include "sqlitequery.h"

#include <QString>

class API_EXPORT SqliteReindex : public SqliteQuery
{
    Q_OBJECT

    public:
        SqliteReindex();
        SqliteReindex(const SqliteReindex& other);
        SqliteReindex(const QString& name1, const QString& name2);

        SqliteStatement* clone();

        QString database = QString();
        QString table = QString();

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
