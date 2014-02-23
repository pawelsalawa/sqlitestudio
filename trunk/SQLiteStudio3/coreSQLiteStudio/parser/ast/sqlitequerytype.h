#ifndef SQLITEQUERYTYPE_H
#define SQLITEQUERYTYPE_H

#include "coreSQLiteStudio_global.h"
#include <QString>

enum class SqliteQueryType
{
    UNDEFINED,
    EMPTY,                // still can hold comments
    AlterTable,
    Analyze,
    Attach,
    BeginTrans,
    CommitTrans,
    Copy,
    CreateIndex,
    CreateTable,
    CreateTrigger,
    CreateView,
    CreateVirtualTable,
    Delete,
    Detach,
    DropIndex,
    DropTable,
    DropTrigger,
    DropView,
    Insert,
    Pragma,
    Reindex,
    Release,
    Rollback,
    Savepoint,
    Select,
    Update,
    Vacuum
};

QString API_EXPORT sqliteQueryTypeToString(const SqliteQueryType& type);

#endif // SQLITEQUERYTYPE_H
