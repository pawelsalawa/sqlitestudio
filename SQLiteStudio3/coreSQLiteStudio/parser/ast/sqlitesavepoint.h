#ifndef SQLITESAVEPOINT_H
#define SQLITESAVEPOINT_H

#include "sqlitequery.h"

#include <QString>

class API_EXPORT SqliteSavepoint : public SqliteQuery
{
    public:
        SqliteSavepoint();
        SqliteSavepoint(const SqliteSavepoint& other);
        explicit SqliteSavepoint(const QString& name);

        SqliteStatement* clone();

        QString name = QString::null;

    protected:
        TokenList rebuildTokensFromContents();
};

typedef QSharedPointer<SqliteSavepoint> SqliteSavepointPtr;

#endif // SQLITESAVEPOINT_H
