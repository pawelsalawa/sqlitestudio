#include "formatrelease.h"
#include "parser/ast/sqliterelease.h"

FormatRelease::FormatRelease(SqliteRelease* release) :
    release(release)
{
}

void FormatRelease::formatInternal()
{
    handleExplainQuery(release);
    withKeyword("RELEASE");
    if (release->savepointKw)
        withKeyword("SAVEPOINT");

    withId(release->name).withSemicolon();
}
