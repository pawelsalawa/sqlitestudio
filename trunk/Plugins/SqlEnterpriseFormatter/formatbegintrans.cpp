#include "formatbegintrans.h"
#include "parser/ast/sqlitebegintrans.h"

FormatBeginTrans::FormatBeginTrans(SqliteBeginTrans* bt) :
    bt(bt)
{
}

void FormatBeginTrans::formatInternal()
{
    withKeyword("BEGIN");

    if (bt->type != SqliteBeginTrans::Type::null)
        withKeyword(SqliteBeginTrans::typeToString(bt->type));

    if (bt->transactionKw)
    {
        withKeyword("TRANSACTION");
        if (!bt->name.isNull())
            withId(bt->name);
    }

    withConflict(bt->onConflict).withSemicolon();
}
