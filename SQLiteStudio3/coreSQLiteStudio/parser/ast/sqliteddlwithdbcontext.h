#ifndef SQLITEDDLWITHDBCONTEXT_H
#define SQLITEDDLWITHDBCONTEXT_H

#include "coreSQLiteStudio_global.h"
#include <QSharedPointer>

class API_EXPORT SqliteDdlWithDbContext
{
    public:
        virtual QString getTargetDatabase() const = 0;
        virtual void setTargetDatabase(const QString& database) = 0;

        virtual QString getObjectName() const = 0;
        virtual void setObjectName(const QString& name) = 0;
};

typedef QSharedPointer<SqliteDdlWithDbContext> SqliteDdlWithDbContextPtr;

#endif // SQLITEDDLWITHDBCONTEXT_H
