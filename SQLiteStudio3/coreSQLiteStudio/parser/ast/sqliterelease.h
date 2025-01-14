#ifndef SQLITERELEASE_H
#define SQLITERELEASE_H

#include "sqlitequery.h"

#include <QString>

class API_EXPORT SqliteRelease : public SqliteQuery
{
    Q_OBJECT

    public:
        SqliteRelease();
        SqliteRelease(const SqliteRelease& other);
        SqliteRelease(bool savepointKw, const QString &name);

        SqliteStatement* clone();

        QString name = QString();
        bool savepointKw = false;

    protected:
        TokenList rebuildTokensFromContents();
};

typedef QSharedPointer<SqliteRelease> SqliteReleasePtr;

#endif // SQLITERELEASE_H
