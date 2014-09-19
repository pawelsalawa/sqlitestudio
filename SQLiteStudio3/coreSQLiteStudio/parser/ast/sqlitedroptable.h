#ifndef SQLITEDROPTABLE_H
#define SQLITEDROPTABLE_H

#include "sqlitequery.h"
#include <QString>

class API_EXPORT SqliteDropTable : public SqliteQuery
{
    public:
        SqliteDropTable();
        SqliteDropTable(bool ifExistsKw, const QString& name1, const QString& name2);

        bool ifExistsKw = false;
        QString database = QString::null;
        QString table = QString::null;

    protected:
        QStringList getTablesInStatement();
        QStringList getDatabasesInStatement();
        TokenList getTableTokensInStatement();
        TokenList getDatabaseTokensInStatement();
        QList<FullObject> getFullObjectsInStatement();
        TokenList rebuildTokensFromContents();
};

typedef QSharedPointer<SqliteDropTable> SqliteDropTablePtr;

#endif // SQLITEDROPTABLE_H
