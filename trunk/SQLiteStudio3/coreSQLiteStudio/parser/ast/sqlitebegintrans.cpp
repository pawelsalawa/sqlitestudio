#include "sqlitebegintrans.h"
#include "sqlitequerytype.h"

SqliteBeginTrans::SqliteBeginTrans()
{
    queryType = SqliteQueryType::BeginTrans;
}

SqliteBeginTrans::SqliteBeginTrans(SqliteBeginTrans::Type type, bool transactionKw, const QString& name)
    : SqliteBeginTrans()
{
    this->type = type;
    this->transactionKw = transactionKw;
    this->name = name;
}

SqliteBeginTrans::SqliteBeginTrans(bool transactionKw, const QString &name, SqliteConflictAlgo onConflict)
{
    this->onConflict = onConflict;
    this->transactionKw = transactionKw;
    this->name = name;
}
