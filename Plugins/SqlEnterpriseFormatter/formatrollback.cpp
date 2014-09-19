#include "formatrollback.h"
#include "parser/ast/sqliterollback.h"

FormatRollback::FormatRollback(SqliteRollback* rollback) :
    rollback(rollback)
{
}

void FormatRollback::formatInternal()
{
    withKeyword("ROLLBACK");
    if (rollback->transactionKw)
        withKeyword("TRANSACTION");

    if (!rollback->name.isNull())
    {
        withKeyword("TO");
        if (rollback->savepointKw)
            withKeyword("SAVEPOINT");

        withId(rollback->name);
    }
    withSemicolon();
}
