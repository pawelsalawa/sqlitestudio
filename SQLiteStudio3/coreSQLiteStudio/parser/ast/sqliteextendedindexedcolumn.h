#ifndef SQLITEEXTENDEDINDEXEDCOLUMN_H
#define SQLITEEXTENDEDINDEXEDCOLUMN_H

#include "coreSQLiteStudio_global.h"
#include <QSharedPointer>

class API_EXPORT SqliteExtendedIndexedColumn
{
    public:
        virtual QString getColumnName() const = 0;
        virtual void setColumnName(const QString& name) = 0;
        virtual QString getCollation() const = 0;
        virtual void setCollation(const QString& name) = 0;
        virtual void clearCollation() = 0;
};

typedef QSharedPointer<SqliteExtendedIndexedColumn> SqliteExtendedIndexedColumnPtr;

#endif // SQLITEEXTENDEDINDEXEDCOLUMN_H
