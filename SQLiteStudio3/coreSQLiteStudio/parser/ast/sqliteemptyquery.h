#ifndef SQLITEEMPTYQUERY_H
#define SQLITEEMPTYQUERY_H

#include "sqlitequery.h"

class API_EXPORT SqliteEmptyQuery : public SqliteQuery
{
    Q_OBJECT

    public:
        SqliteEmptyQuery();
        SqliteEmptyQuery(const SqliteEmptyQuery& other);

        SqliteStatement* clone();

        TokenList rebuildTokensFromContents();
};

typedef QSharedPointer<SqliteEmptyQuery> SqliteEmptyQueryPtr;

#endif // SQLITEEMPTYQUERY_H
