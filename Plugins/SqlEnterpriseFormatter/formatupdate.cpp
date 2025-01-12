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
    handleExplainQuery(upd);
    if (upd->with)
        withStatement(upd->with);

    markKeywordLineUp("UPDATE");
    withKeyword("UPDATE");
    if (upd->onConflict != SqliteConflictAlgo::null)
        withKeyword("OR").withKeyword(sqliteConflictAlgo(upd->onConflict));

    if (!upd->database.isNull())
        withId(upd->database).withIdDot();

    withId(upd->table);
    if (!upd->tableAlias.isNull())
        withKeyword("AS").withId(upd->tableAlias);

    if (upd->indexedByKw)
        withKeyword("INDEXED").withKeyword("BY").withId(upd->indexedBy);
    else if (upd->notIndexedKw)
        withKeyword("NOT").withKeyword("INDEXED");

    withNewLine().withLinedUpKeyword("SET");

    markAndKeepIndent("updateColumns");

    bool first = true;
    for (const SqliteUpdate::ColumnAndValue& keyVal : upd->keyValueMap)
    {
        if (!first)
            withListComma();

        if (keyVal.first.userType() == QMetaType::QStringList)
            withParDefLeft().withIdList(keyVal.first.toStringList()).withParDefRight().withOperator("=").withStatement(keyVal.second);
        else
            withId(keyVal.first.toString()).withOperator("=").withStatement(keyVal.second);

        first = false;
    }

    withDecrIndent();

    if (upd->from)
        withNewLine().withLinedUpKeyword("FROM").withStatement(upd->from, "updateColumns");

    if (upd->where)
        withNewLine().withLinedUpKeyword("WHERE").withStatement(upd->where);

    if (!upd->returning.isEmpty())
    {
        withNewLine().withLinedUpKeyword("RETURNING");
        withStatementList(upd->returning, "returningColumns");
    }

    if (upd->orderBy.size() > 0)
        withNewLine().withLinedUpKeyword("ORDER").withKeyword("BY").withStatementList(upd->orderBy, "order");

    if (upd->limit)
        withNewLine().withLinedUpKeyword("LIMIT").withStatement(upd->limit, "limit");

    withSemicolon();
}
