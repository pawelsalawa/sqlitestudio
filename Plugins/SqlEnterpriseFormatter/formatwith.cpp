#include "formatwith.h"
#include "parser/ast/sqliteselect.h"

FormatWith::FormatWith(SqliteWith *with) :
    with(with)
{
}

void FormatWith::formatInternal()
{
    markKeywordLineUp("SELECT");

    withLinedUpKeyword("WITH");
    if (with->recursive)
        withKeyword("RECURSIVE");

    withStatementList(with->cteList);
}


FormatWithCommonTableExpression::FormatWithCommonTableExpression(SqliteWith::CommonTableExpression *cte) :
    cte(cte)
{
}

void FormatWithCommonTableExpression::formatInternal()
{
    withId(cte->table).withParDefLeft().withStatementList(cte->indexedColumns, "idxCols").withParDefRight()
            .withKeyword("AS").withParDefLeft().withStatement(cte->select).withParDefRight();
}
