#include "formatdelete.h"
#include "parser/ast/sqlitedelete.h"
#include "parser/ast/sqliteexpr.h"
#include "formatwith.h"

FormatDelete::FormatDelete(SqliteDelete* del) :
    del(del)
{
}

void FormatDelete::formatInternal()
{
    handleExplainQuery(del);
    if (del->with)
        withStatement(del->with);

    markKeywordLineUp("DELETE FROM");

    withKeyword("DELETE").withKeyword("FROM");
    if (!del->database.isNull())
        withId(del->database).withIdDot();

    withId(del->table);

    if (del->indexedByKw)
        withKeyword("INDEXED").withKeyword("BY").withId(del->indexedBy);
    else if (del->notIndexedKw)
        withKeyword("NOT").withKeyword("INDEXED");

    if (del->where)
        withNewLine().withLinedUpKeyword("WHERE").withStatement(del->where);

    if (!del->returning.isEmpty())
    {
        withNewLine().withLinedUpKeyword("RETURNING");
        withStatementList(del->returning, "returningColumns");
    }

    withSemicolon();
}
