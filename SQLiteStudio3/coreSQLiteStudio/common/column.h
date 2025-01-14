#ifndef COLUMN_H
#define COLUMN_H

#include "table.h"
#include "coreSQLiteStudio_global.h"
#include <QString>

struct API_EXPORT Column : public Table
{
    public:
        Column() = default;
        Column(const QString& database, const QString& table, const QString& column);
        Column(const Column& other) = default;

        bool operator ==(const Column& other) const = default;
        Column& operator=(const Column&) = default;
        Column& operator=(Column&&) = default;

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
        AliasedColumn() = default;
        AliasedColumn(const QString& database, const QString& table, const QString& column, const QString& alias);
        AliasedColumn(const AliasedColumn& other) = default;

        bool operator ==(const AliasedColumn& other) const = default;
        AliasedColumn& operator=(const AliasedColumn&) = default;
        AliasedColumn& operator=(AliasedColumn&&) = default;

        QString getAlias() const;
        void setAlias(const QString& value);

    private:
        QString alias;
};

API_EXPORT size_t qHash(Column column);
API_EXPORT size_t qHash(AliasedColumn column);

#endif // COLUMN_H
