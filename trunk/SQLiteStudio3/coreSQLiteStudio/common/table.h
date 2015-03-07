#ifndef TABLE_H
#define TABLE_H

#include "coreSQLiteStudio_global.h"
#include <QString>

class API_EXPORT Table
{
    public:
        Table();
        Table(const QString& database, const QString& table);
        Table(const Table& other);
        virtual ~Table();

        int operator ==(const Table& other) const;

        QString getDatabase() const;
        void setDatabase(const QString& value);

        QString getTable() const;
        void setTable(const QString& value);

    protected:
        QString database;
        QString table;
};

class API_EXPORT AliasedTable : public Table
{
    public:
        AliasedTable();
        AliasedTable(const QString& database, const QString& table, const QString& alias);
        AliasedTable(const AliasedTable& other);
        virtual ~AliasedTable();

        int operator ==(const AliasedTable& other) const;

        QString getTableAlias() const;
        void setTableAlias(const QString& value);

    protected:
        QString tableAlias;
};

int API_EXPORT qHash(Table table);
int API_EXPORT qHash(AliasedTable table);


#endif // TABLE_H
