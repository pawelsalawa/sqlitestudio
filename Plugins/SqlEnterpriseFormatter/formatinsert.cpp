#include "formatinsert.h"
#include "parser/ast/sqliteselect.h"
#include "parser/ast/sqliteinsert.h"
#include "parser/ast/sqliteupsert.h"
#include "formatwith.h"

FormatInsert::FormatInsert(SqliteInsert* insert) :
    insert(insert)
{
}

void FormatInsert::formatInternal()
{
    handleExplainQuery(insert);
    if (insert->replaceKw)
    {
        withStatement(insert->with);
        withKeyword("REPLACE");
    }
    else
    {
        withStatement(insert->with);
        withKeyword("INSERT");
        if (insert->onConflict != SqliteConflictAlgo::null)
            withKeyword("OR").withKeyword(sqliteConflictAlgo(insert->onConflict));
    }

    withKeyword("INTO");

    if (!insert->database.isNull())
        withId(insert->database);

    withId(insert->table);
    if (!insert->tableAlias.isNull())
        withKeyword("AS").withId(insert->tableAlias);

    if (insert->defaultValuesKw)
    {
        withKeyword("DEFAULT").withKeyword("VALUES");
    }
    else
    {
        markAndKeepIndent("insertCols");
        if (insert->columnNames.size() > 0)
            withParDefLeft().withIdList(insert->columnNames).withParDefRight();

        if (insert->select)
        {
            withStatement(insert->select);
        }
        if (insert->upsert)
            withStatement(insert->upsert);

        withDecrIndent();
    }

    if (!insert->returning.isEmpty())
    {
        withNewLine().withLinedUpKeyword("RETURNING");
        withStatementList(insert->returning, "returningColumns");
    }
    withSemicolon();
}
