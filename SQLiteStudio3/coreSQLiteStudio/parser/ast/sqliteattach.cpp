#include "sqliteattach.h"
#include "sqlitequerytype.h"
#include "sqliteexpr.h"
#include "parser/statementtokenbuilder.h"

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

TokenList SqliteAttach::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;

    builder.withKeyword("ATTACH").withSpace();

    if (databaseKw)
        builder.withKeyword("DATABASE").withSpace();

    builder.withStatement(databaseUrl).withSpace().withKeyword("AS").withSpace().withStatement(name);
    if (key)
        builder.withSpace().withKeyword("KEY").withSpace().withStatement(key);

    builder.withOperator(";");

    return builder.build();
}
