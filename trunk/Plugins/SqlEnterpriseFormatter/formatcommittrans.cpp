#include "formatcommittrans.h"
#include "parser/ast/sqlitecommittrans.h"

FormatCommitTrans::FormatCommitTrans(SqliteCommitTrans* ct) :
    ct(ct)
{
}

void FormatCommitTrans::formatInternal()
{
    handleExplainQuery(ct);
    if (ct->endKw)
        withKeyword("END");
    else
        withKeyword("COMMIT");

    if (ct->transactionKw)
    {
        withKeyword("TRANSACTION");
        if (!ct->name.isNull())
            withId(ct->name);
    }

    withOperator(";");
}
