#include "sqliteattach.h"
#include "sqlitequerytype.h"
#include "sqliteexpr.h"

SqliteAttach::SqliteAttach()
{
    queryType = SqliteQueryType::Attach;
}

SqliteAttach::SqliteAttach(bool dbKw, SqliteExpr *url, SqliteExpr *name, SqliteExpr *key)
    : SqliteAttach()
{
    databaseKw = dbKw;
    databaseUrl = url;
    this->name = name;
    this->key = key;

    if (databaseUrl)
        databaseUrl->setParent(this);

    if (name)
        name->setParent(this);

    if (key)
        key->setParent(this);
}

SqliteAttach::~SqliteAttach()
{
}
