#ifndef SQLITEVACUUM_H
#define SQLITEVACUUM_H

#include "sqlitequery.h"

#include <QString>

class SqliteExpr;

class API_EXPORT SqliteVacuum : public SqliteQuery
{
    Q_OBJECT

    public:
        SqliteVacuum();
        SqliteVacuum(const SqliteVacuum& other);
        SqliteVacuum(SqliteExpr* expr);
        SqliteVacuum(const QString &name, SqliteExpr* expr);

        SqliteStatement* clone();

        QString database;
        SqliteExpr* expr = nullptr;

    protected:
        QStringList getDatabasesInStatement();
        TokenList getDatabaseTokensInStatement();
        QList<FullObject> getFullObjectsInStatement();
        TokenList rebuildTokensFromContents();
};

typedef QSharedPointer<SqliteVacuum> SqliteVacuumPtr;

#endif // SQLITEVACUUM_H
