#ifndef SQLITEDROPINDEX_H
#define SQLITEDROPINDEX_H

#include "sqlitequery.h"
#include <QString>

class API_EXPORT SqliteDropIndex : public SqliteQuery
{
    Q_OBJECT

    public:
        SqliteDropIndex();
        SqliteDropIndex(const SqliteDropIndex& other);
        SqliteDropIndex(bool ifExistsKw, const QString& name1, const QString& name2);

        SqliteStatement* clone();

        bool ifExistsKw = false;
        QString database = QString();
        QString index = QString();

    protected:
        QStringList getDatabasesInStatement();
        TokenList getDatabaseTokensInStatement();
        QList<FullObject> getFullObjectsInStatement();
        TokenList rebuildTokensFromContents();
};

typedef QSharedPointer<SqliteDropIndex> SqliteDropIndexPtr;

#endif // SQLITEDROPINDEX_H
