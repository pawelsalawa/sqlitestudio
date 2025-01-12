#ifndef SQLITEDROPTABLE_H
#define SQLITEDROPTABLE_H

#include "sqlitequery.h"
#include <QString>

class API_EXPORT SqliteDropTable : public SqliteQuery
{
    Q_OBJECT

    public:
        SqliteDropTable();
        SqliteDropTable(const SqliteDropTable& other);
        SqliteDropTable(bool ifExistsKw, const QString& name1, const QString& name2);

        SqliteStatement* clone();

        bool ifExistsKw = false;
        QString database = QString();
        QString table = QString();

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
