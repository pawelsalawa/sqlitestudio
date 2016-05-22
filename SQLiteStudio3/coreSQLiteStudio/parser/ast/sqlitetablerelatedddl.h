#ifndef SQLITETABLERELATEDDDL_H
#define SQLITETABLERELATEDDDL_H

#include "coreSQLiteStudio_global.h"
#include <QSharedPointer>

class API_EXPORT SqliteTableRelatedDdl
{
    public:
        virtual QString getTargetTable() const = 0;
};

typedef QSharedPointer<SqliteTableRelatedDdl> SqliteTableRelatedDdlPtr;

#endif // SQLITETABLERELATEDDDL_H
