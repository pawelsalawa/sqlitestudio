#ifndef SQLITERELEASE_H
#define SQLITERELEASE_H

#include "sqlitequery.h"

#include <QString>

class API_EXPORT SqliteRelease : public SqliteQuery
{
    public:
        SqliteRelease();
        SqliteRelease(bool savepointKw, const QString &name);

        QString name = QString::null;
        bool savepointKw = false;
};

typedef QSharedPointer<SqliteRelease> SqliteReleasePtr;

#endif // SQLITERELEASE_H
