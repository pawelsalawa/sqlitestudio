#include "sqlitesavepoint.h"
#include "sqlitequerytype.h"

#include <parser/statementtokenbuilder.h>

SqliteSavepoint::SqliteSavepoint()
{
    queryType = SqliteQueryType::Savepoint;
}

SqliteSavepoint::SqliteSavepoint(const QString &name)
    : SqliteSavepoint()
{
    this->name = name;
}

TokenList SqliteSavepoint::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;
    builder.withKeyword("SAVEPOINT").withSpace().withOther(name, dialect).withOperator(";");
    return builder.build();
}
