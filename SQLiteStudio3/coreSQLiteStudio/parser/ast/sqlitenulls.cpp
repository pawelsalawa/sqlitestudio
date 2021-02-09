#include "sqlitenulls.h"


SqliteNulls sqliteNulls(const QString& value)
{
    if (value == "NULLS FIRST")
        return SqliteNulls::FIRST;
    else if (value == "NULLS LAST")
        return SqliteNulls::LAST;
    else
        return SqliteNulls::null;
}

QString sqliteNulls(SqliteNulls value)
{
    switch (value)
    {
        case SqliteNulls::FIRST:
            return "FIRST";
        case SqliteNulls::LAST:
            return "LAST";
        case SqliteNulls::null:
            break;
    }
    return QString();
}
