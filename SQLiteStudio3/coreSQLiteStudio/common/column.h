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

        QString getDeclaredType() const;
        void setDeclaredType(const QString& value);

    private:
        QString column;
        QString declaredType;
};

struct API_EXPORT AliasedColumn : public Column
{
    public:
        AliasedColumn();
        AliasedColumn(const QString& database, const QString& table, const QString& column, const QString& alias);
        AliasedColumn(const AliasedColumn& other);

        int operator ==(const AliasedColumn& other) const;

        QString getAlias() const;
        void setAlias(const QString& value);

    private:
        QString alias;
};

int API_EXPORT qHash(Column column);
int API_EXPORT qHash(AliasedColumn column);

#endif // COLUMN_H
