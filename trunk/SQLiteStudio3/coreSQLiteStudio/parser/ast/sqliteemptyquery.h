#ifndef SQLITEEMPTYQUERY_H
#define SQLITEEMPTYQUERY_H

#include "sqlitequery.h"

class API_EXPORT SqliteEmptyQuery : public SqliteQuery
{
    public:
        SqliteEmptyQuery();
};

typedef QSharedPointer<SqliteEmptyQuery> SqliteEmptyQueryPtr;

#endif // SQLITEEMPTYQUERY_H
