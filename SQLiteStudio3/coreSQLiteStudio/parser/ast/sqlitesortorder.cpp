#include "sqlitesortorder.h"

SqliteSortOrder sqliteSortOrder(const QString& value)
{
    if (value == "ASC")
        return SqliteSortOrder::ASC;
    else if (value == "DESC")
        return SqliteSortOrder::DESC;
    else
        return SqliteSortOrder::null;
}

QString sqliteSortOrder(SqliteSortOrder value)
{
    switch (value)
    {
        case SqliteSortOrder::ASC:
            return "ASC";
        case SqliteSortOrder::DESC:
            return "DESC";
        default:
            return QString::null;

    }
}
