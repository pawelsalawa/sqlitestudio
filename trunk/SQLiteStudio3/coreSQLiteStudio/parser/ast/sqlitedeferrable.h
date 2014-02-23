#ifndef SQLITEDEFERRABLE_H
#define SQLITEDEFERRABLE_H

#include "coreSQLiteStudio_global.h"
#include <QString>

enum class SqliteDeferrable
{
    null,
    NOT_DEFERRABLE,
    DEFERRABLE
};

enum class SqliteInitially
{
    null,
    DEFERRED,
    IMMEDIATE
};

API_EXPORT QString sqliteDeferrable(SqliteDeferrable deferrable);
API_EXPORT SqliteDeferrable sqliteDeferrable(const QString& deferrable);

API_EXPORT QString sqliteInitially(SqliteInitially initially);
API_EXPORT SqliteInitially sqliteInitially(const QString& initially);

#endif // SQLITEDEFERRABLE_H
