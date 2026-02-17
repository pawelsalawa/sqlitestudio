#include "formatsavepoint.h"
#include "parser/ast/sqlitesavepoint.h"

FormatSavepoint::FormatSavepoint(SqliteSavepoint* savepoint) :
    FormatStatement(savepoint), savepoint(savepoint)
{
}

void FormatSavepoint::formatInternal()
{
    handleExplainQuery(savepoint);
    withKeyword("SAVEPOINT").withId(savepoint->name).withSemicolon();
}
