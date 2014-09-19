#include "formatdropview.h"
#include "parser/ast/sqlitedropview.h"

FormatDropView::FormatDropView(SqliteDropView* dropView) :
    dropView(dropView)
{
}

void FormatDropView::formatInternal()
{
    withKeyword("DROP").withKeyword("VIEW");

    if (dropView->ifExistsKw)
        withKeyword("IF").withKeyword("EXISTS");

    if (!dropView->database.isNull())
        withId(dropView->database).withIdDot();

    withId(dropView->view).withSemicolon();
}
