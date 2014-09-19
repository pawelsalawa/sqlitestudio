#include "formatdetach.h"
#include "parser/ast/sqlitedetach.h"
#include "parser/ast/sqliteexpr.h"

FormatDetach::FormatDetach(SqliteDetach* detach) :
    detach(detach)
{
}

void FormatDetach::formatInternal()
{
    withKeyword("DETACH");

    if (detach->databaseKw)
        withKeyword("DATABASE");

    withStatement(detach->name).withSemicolon();
}
