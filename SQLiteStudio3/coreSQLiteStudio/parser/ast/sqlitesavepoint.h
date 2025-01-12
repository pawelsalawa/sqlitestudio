#ifndef SQLITESAVEPOINT_H
#define SQLITESAVEPOINT_H

#include "sqlitequery.h"

#include <QString>

class API_EXPORT SqliteSavepoint : public SqliteQuery
{
    Q_OBJECT

    public:
        SqliteSavepoint();
        SqliteSavepoint(const SqliteSavepoint& other);
        explicit SqliteSavepoint(const QString& name);

        SqliteStatement* clone();

        QString name = QString();

    protected:
        TokenList rebuildTokensFromContents();
};

typedef QSharedPointer<SqliteSavepoint> SqliteSavepointPtr;

#endif // SQLITESAVEPOINT_H
