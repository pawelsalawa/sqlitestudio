#include "formatdropindex.h"
#include "parser/ast/sqlitedropindex.h"

FormatDropIndex::FormatDropIndex(SqliteDropIndex* dropIndex) :
    dropIndex(dropIndex)
{
}

void FormatDropIndex::formatInternal()
{
    withKeyword("DROP").withKeyword("INDEX");

    if (dropIndex->ifExistsKw)
        withKeyword("IF").withKeyword("EXISTS");

    if (!dropIndex->database.isNull())
        withId(dropIndex->database).withIdDot();

    withId(dropIndex->index).withSemicolon();
}
