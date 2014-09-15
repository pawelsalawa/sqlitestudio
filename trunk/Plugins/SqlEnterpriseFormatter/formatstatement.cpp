#include "formatstatement.h"
#include "formatselect.h"
#include "parser/ast/sqliteselect.h"
#include <QDebug>

#define FORMATTER_FACTORY_ENTRY(query, Type) \
    Sqlite##Type* local##Type##Ptr = dynamic_cast<Sqlite##Type*>(query); \
    if (local##Type##Ptr) \
        return new Format##Type(local##Type##Ptr)

FormatStatement::FormatStatement()
{
}

FormatStatement *FormatStatement::forQuery(SqliteQuery* query)
{
    FORMATTER_FACTORY_ENTRY(query, Select);

    qWarning() << "Unhandled query passed to enterprise formatter!";
    return nullptr;
}
