#include "formatcreateview.h"
#include "parser/ast/sqlitecreateview.h"
#include "parser/ast/sqliteselect.h"

FormatCreateView::FormatCreateView(SqliteCreateView* createView) :
    createView(createView)
{
}

void FormatCreateView::formatInternal()
{
    withKeyword("CREATE");
    if (createView->tempKw)
        withKeyword("TEMP");
    else if (createView->temporaryKw)
        withKeyword("TEMPORARY");

    withKeyword("VIEW");
    if (createView->ifNotExists)
        withKeyword("IF").withKeyword("NOT").withKeyword("EXISTS");

    if (dialect == Dialect::Sqlite3 && !createView->database.isNull())
        withId(createView->database).withIdDot();

    withId(createView->view).withKeyword("AS").withNewLine().withIncrIndent().withStatement(createView->select).withSemicolon().withDecrIndent();
}
