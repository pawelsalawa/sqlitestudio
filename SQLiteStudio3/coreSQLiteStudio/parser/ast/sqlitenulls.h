#ifndef SQLITENULLS_H
#define SQLITENULLS_H

#include "coreSQLiteStudio_global.h"
#include <QString>

enum class SqliteNulls
{
    FIRST,
    LAST,
    null
};

API_EXPORT SqliteNulls sqliteNulls(const QString& value);
API_EXPORT QString sqliteNulls(SqliteNulls value);

#endif // SQLITENULLS_H
