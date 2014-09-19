#include "sqlitecommittrans.h"
#include "sqlitequerytype.h"

#include <parser/statementtokenbuilder.h>

SqliteCommitTrans::SqliteCommitTrans()
{
    queryType = SqliteQueryType::CommitTrans;
}

SqliteCommitTrans::SqliteCommitTrans(bool transactionKw, const QString& name, bool endKw)
    : SqliteCommitTrans()
{
    this->endKw = endKw;
    this->transactionKw = transactionKw;
    this->name = name;
}

TokenList SqliteCommitTrans::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;

    if (endKw)
        builder.withKeyword("END");
    else
        builder.withKeyword("COMMIT");

    if (transactionKw)
    {
        builder.withSpace().withKeyword("TRANSACTION");
        if (!name.isNull())
            builder.withSpace().withOther(name, dialect);
    }

    builder.withOperator(";");

    return builder.build();
}
