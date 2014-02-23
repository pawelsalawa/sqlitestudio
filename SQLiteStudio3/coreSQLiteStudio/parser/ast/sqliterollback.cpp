#include "sqliterollback.h"
#include "sqlitequerytype.h"

SqliteRollback::SqliteRollback()
{
    queryType = SqliteQueryType::Rollback;
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
