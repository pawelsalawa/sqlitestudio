#include "sqlitedeferrable.h"

QString sqliteDeferrable(SqliteDeferrable deferrable)
{
    switch (deferrable)
    {
        case SqliteDeferrable::NOT_DEFERRABLE:
            return "NOT DEFERRABLE";
        case SqliteDeferrable::DEFERRABLE:
            return "DEFERRABLE";
        case SqliteDeferrable::null:
            break;
    }
    return QString::null;
}

SqliteDeferrable sqliteDeferrable(const QString& deferrable)
{
    QString upper = deferrable.toUpper();
    if (upper == "NOT DEFERRABLE")
        return SqliteDeferrable::NOT_DEFERRABLE;

    if (upper == "DEFERRABLE")
        return SqliteDeferrable::DEFERRABLE;

    return SqliteDeferrable::null;
}


QString sqliteInitially(SqliteInitially initially)
{
    switch (initially)
    {
        case SqliteInitially::DEFERRED:
            return "DEFERRED";
        case SqliteInitially::IMMEDIATE:
            return "IMMEDIATE";
        case SqliteInitially::null:
            break;
    }
    return QString::null;
}

SqliteInitially sqliteInitially(const QString& initially)
{
    QString upper = initially.toUpper();
    if (upper == "DEFERRED")
        return SqliteInitially::DEFERRED;

    if (upper == "IMMEDIATE")
        return SqliteInitially::IMMEDIATE;

    return SqliteInitially::null;
}
