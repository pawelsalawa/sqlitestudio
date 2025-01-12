#ifndef SQLITEQUERYWITHALIASEDTABLE_H
#define SQLITEQUERYWITHALIASEDTABLE_H

#include "coreSQLiteStudio_global.h"
#include <QSharedPointer>

class API_EXPORT SqliteQueryWithAliasedTable
{
    public:
        virtual QString getTable() const = 0;
        virtual QString getDatabase() const = 0;
        virtual QString getTableAlias() const = 0;
};

typedef QSharedPointer<SqliteQueryWithAliasedTable> SqliteQueryWithAliasedTablePtr;

#endif // SQLITEQUERYWITHALIASEDTABLE_H
