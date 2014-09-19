#include "formatattach.h"
#include "parser/ast/sqliteattach.h"
#include "parser/ast/sqliteexpr.h"

FormatAttach::FormatAttach(SqliteAttach* att) :
    att(att)
{
}

void FormatAttach::formatInternal()
{
    withKeyword("ATTACH");

    if (att->databaseKw)
        withKeyword("DATABASE");

    withStatement(att->databaseUrl).withKeyword("AS").withStatement(att->name);
    if (att->key)
        withKeyword("KEY").withStatement(att->key);

    withSemicolon();
}
