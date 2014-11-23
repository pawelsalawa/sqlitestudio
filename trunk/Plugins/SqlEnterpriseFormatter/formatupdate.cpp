#include "formatupdate.h"
#include "parser/ast/sqliteupdate.h"
#include "parser/ast/sqliteexpr.h"
#include "formatwith.h"

FormatUpdate::FormatUpdate(SqliteUpdate* upd) :
    upd(upd)
{
}

void FormatUpdate::formatInternal()
{
    if (upd->with)
        withStatement(upd->with);

    markKeywordLineUp("UPDATE");
    withKeyword("UPDATE");
    if (upd->onConflict != SqliteConflictAlgo::null)
        withKeyword("OR").withKeyword(sqliteConflictAlgo(upd->onConflict));

    if (!upd->database.isNull())
        withId(upd->database).withIdDot();

    withId(upd->table);

    if (upd->indexedByKw)
        withKeyword("INDEXED").withKeyword("BY").withId(upd->indexedBy);
    else if (upd->notIndexedKw)
        withKeyword("NOT").withKeyword("INDEXED");

    withNewLine().withLinedUpKeyword("SET");

    bool first = true;
    foreach (const SqliteUpdate::ColumnAndValue& keyVal, upd->keyValueMap)
    {
        if (!first)
            withListComma();

        withId(keyVal.first).withOperator("=").withStatement(keyVal.second);
        first = false;
    }

    if (upd->where)
        withNewLine().withLinedUpKeyword("WHERE").withStatement(upd->where);

    withSemicolon();
}
