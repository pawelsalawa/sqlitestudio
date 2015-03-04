#include "sqliterollback.h"
#include "sqlitequerytype.h"

#include <parser/statementtokenbuilder.h>

SqliteRollback::SqliteRollback()
{
    queryType = SqliteQueryType::Rollback;
}

SqliteRollback::SqliteRollback(const SqliteRollback& other) :
    SqliteQuery(other), transactionKw(other.transactionKw), toKw(other.toKw), savepointKw(other.savepointKw), name(other.name)
{
}

SqliteRollback::SqliteRollback(bool transactionKw, const QString& name)
    : SqliteRollback()
{
    this->name = name;
    this->transactionKw = transactionKw;
}

SqliteRollback::SqliteRollback(bool transactionKw, bool savePoint, const QString& name)
{
    // we ignore name from trans_opt,
    // it's not officialy supported in sqlite3
    this->name = name;
    this->transactionKw = transactionKw;
    toKw = true;
    savepointKw = savePoint;
}

SqliteStatement*SqliteRollback::clone()
{
    return new SqliteRollback(*this);
}

TokenList SqliteRollback::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;
    builder.withTokens(SqliteQuery::rebuildTokensFromContents());
    builder.withKeyword("ROLLBACK").withSpace();
    if (transactionKw)
        builder.withKeyword("TRANSACTION").withSpace();

    if (!name.isNull())
    {
        builder.withKeyword("TO").withSpace();
        if (savepointKw)
            builder.withKeyword("SAVEPOINT").withSpace();

        builder.withOther(name, dialect);
    }
    builder.withOperator(";");

    return builder.build();
}
