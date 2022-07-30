#include "formatwith.h"
#include "parser/ast/sqliteselect.h"

FormatWith::FormatWith(SqliteWith *with) :
    with(with)
{
}

void FormatWith::setLineUpKeyword(const QString& kw)
{
    lineUpKeyword = kw;
}

void FormatWith::formatInternal()
{
    markKeywordLineUp(lineUpKeyword);

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
    withId(cte->table);
    if (cte->indexedColumns.size() > 0)
        withParDefLeft().withStatementList(cte->indexedColumns, "idxCols").withParDefRight();

    withKeyword("AS");
    switch (cte->asMode) {
        case SqliteWith::CommonTableExpression::ANY:
            break;
        case SqliteWith::CommonTableExpression::MATERIALIZED:
            withKeyword("MATERIALIZED");
            break;
        case SqliteWith::CommonTableExpression::NOT_MATERIALIZED:
            withKeyword("NOT").withKeyword("MATERIALIZED");
            break;
    }
    withParDefLeft().withStatement(cte->select).withParDefRight();
}
