#ifndef SQLITEVACUUM_H
#define SQLITEVACUUM_H

#include "sqlitequery.h"

#include <QString>

class API_EXPORT SqliteVacuum : public SqliteQuery
{
    public:
        SqliteVacuum();
        explicit SqliteVacuum(const QString &name);

        QString database;

    protected:
        QStringList getDatabasesInStatement();
        TokenList getDatabaseTokensInStatement();
        QList<FullObject> getFullObjectsInStatement();
        TokenList rebuildTokensFromContents();
};

typedef QSharedPointer<SqliteVacuum> SqliteVacuumPtr;

#endif // SQLITEVACUUM_H
