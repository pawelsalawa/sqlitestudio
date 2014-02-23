#ifndef SQLITECONFLICTALGO_H
#define SQLITECONFLICTALGO_H

#include "coreSQLiteStudio_global.h"
#include <QString>

enum class SqliteConflictAlgo
{
    ROLLBACK,
    ABORT,
    FAIL,
    IGNORE,
    REPLACE,
    null
};

API_EXPORT SqliteConflictAlgo sqliteConflictAlgo(const QString& value);
API_EXPORT QString sqliteConflictAlgo(SqliteConflictAlgo value);

#endif // SQLITECONFLICTALGO_H
