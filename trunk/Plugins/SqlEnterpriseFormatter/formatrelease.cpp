#include "formatrelease.h"
#include "parser/ast/sqliterelease.h"

FormatRelease::FormatRelease(SqliteRelease* release) :
    release(release)
{
}

void FormatRelease::formatInternal()
{
    withKeyword("RELEASE");
    if (release->savepointKw)
        withKeyword("SAVEPOINT");

    withId(release->name).withSemicolon();
}
