#include "formatcopy.h"
#include "parser/ast/sqlitecopy.h"

FormatCopy::FormatCopy(SqliteCopy* copy) :
    copy(copy)
{
}

void FormatCopy::formatInternal()
{
    handleExplainQuery(copy);
    withKeyword("COPY");
    if (copy->onConflict != SqliteConflictAlgo::null)
        withKeyword("OR").withKeyword(sqliteConflictAlgo(copy->onConflict));

    if (!copy->database.isNull())
        withId(copy->database);

    withId(copy->table).withKeyword("FROM").withString(copy->file);

    if (!copy->delimiter.isNull())
        withKeyword("USING").withKeyword("DELIMITERS").withString(copy->delimiter);

    withSemicolon();
}
