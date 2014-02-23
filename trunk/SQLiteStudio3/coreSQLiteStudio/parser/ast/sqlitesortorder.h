#ifndef SQLITESORTORDER_H
#define SQLITESORTORDER_H

#include "coreSQLiteStudio_global.h"
#include <QString>

enum class SqliteSortOrder
{
    ASC,
    DESC,
    null
};

API_EXPORT SqliteSortOrder sqliteSortOrder(const QString& value);
API_EXPORT QString sqliteSortOrder(SqliteSortOrder value);

#endif // SQLITESORTORDER_H
