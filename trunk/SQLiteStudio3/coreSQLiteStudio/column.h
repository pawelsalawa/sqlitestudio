#ifndef COLUMN_H
#define COLUMN_H

#include "table.h"
#include "coreSQLiteStudio_global.h"
#include <QString>

struct API_EXPORT Column : public Table
{
    public:
        Column();
        Column(const QString& database, const QString& table, const QString& column);
        Column(const Column& other);

        int operator ==(const Column& other) const;

        QString getColumn() const;
        void setColumn(const QString& value);

    private:
        QString column;
};

int API_EXPORT qHash(Column column);

#endif // COLUMN_H
