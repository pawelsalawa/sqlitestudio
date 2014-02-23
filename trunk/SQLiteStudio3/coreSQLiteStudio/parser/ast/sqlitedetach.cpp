#include "sqlitedetach.h"
#include "sqlitequerytype.h"
#include "sqliteexpr.h"

SqliteDetach::SqliteDetach()
{
    queryType = SqliteQueryType::Detach;
}

SqliteDetach::SqliteDetach(bool databaseKw, SqliteExpr *name)
    :SqliteDetach()
{
    this->databaseKw = databaseKw;
    this->name = name;
    if (name)
        name->setParent(this);
}

SqliteDetach::~SqliteDetach()
{
//    if (name)
//        delete name;
}
