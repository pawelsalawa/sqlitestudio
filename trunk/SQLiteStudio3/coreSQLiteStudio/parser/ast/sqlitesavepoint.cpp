#include "sqlitesavepoint.h"
#include "sqlitequerytype.h"

SqliteSavepoint::SqliteSavepoint()
{
    queryType = SqliteQueryType::Savepoint;
}

SqliteSavepoint::SqliteSavepoint(const QString &name)
    : SqliteSavepoint()
{
    this->name = name;
}
