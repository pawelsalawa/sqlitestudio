#ifndef SQLITEDROPINDEX_H
#define SQLITEDROPINDEX_H

#include "sqlitequery.h"
#include <QString>

class API_EXPORT SqliteDropIndex : public SqliteQuery
{
    public:
        SqliteDropIndex();
        SqliteDropIndex(bool ifExistsKw, const QString& name1, const QString& name2);

        bool ifExistsKw = false;
        QString database = QString::null;
        QString index = QString::null;

    protected:
        QStringList getDatabasesInStatement();
        TokenList getDatabaseTokensInStatement();
        QList<FullObject> getFullObjectsInStatement();
        TokenList rebuildTokensFromContents();
};

typedef QSharedPointer<SqliteDropIndex> SqliteDropIndexPtr;

#endif // SQLITEDROPINDEX_H
