#include "formatreindex.h"
#include "parser/ast/sqlitereindex.h"

FormatReindex::FormatReindex(SqliteReindex* reindex) :
    reindex(reindex)
{
}

void FormatReindex::formatInternal()
{
    handleExplainQuery(reindex);
    withKeyword("REINDEX");
    if (!reindex->database.isNull())
        withId(reindex->database).withIdDot();

    withId(reindex->table).withSemicolon();
}
