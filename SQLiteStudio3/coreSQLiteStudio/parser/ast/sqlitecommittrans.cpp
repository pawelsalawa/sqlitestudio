#include "sqlitecommittrans.h"
#include "sqlitequerytype.h"

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
