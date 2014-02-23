#include "sqliteconflictalgo.h"

SqliteConflictAlgo sqliteConflictAlgo(const QString& value)
{
    QString upper = value.toUpper();
    if (upper == "ROLLBACK")
        return SqliteConflictAlgo::ROLLBACK;
    else if (upper == "ABORT")
        return SqliteConflictAlgo::ABORT;
    else if (upper == "FAIL")
        return SqliteConflictAlgo::FAIL;
    else if (upper == "IGNORE")
        return SqliteConflictAlgo::IGNORE;
    else if (upper == "REPLACE")
        return SqliteConflictAlgo::REPLACE;
    else
        return SqliteConflictAlgo::null;
}

QString sqliteConflictAlgo(SqliteConflictAlgo value)
{
    switch (value)
    {
        case SqliteConflictAlgo::ROLLBACK:
            return "ROLLBACK";
        case SqliteConflictAlgo::ABORT:
            return "ABORT";
        case SqliteConflictAlgo::FAIL:
            return "FAIL";
        case SqliteConflictAlgo::IGNORE:
            return "IGNORE";
        case SqliteConflictAlgo::REPLACE:
            return "REPLACE";
        default:
            return QString::null;
    }
}
