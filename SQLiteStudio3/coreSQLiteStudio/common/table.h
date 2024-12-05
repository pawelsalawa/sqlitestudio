#ifndef TABLE_H
#define TABLE_H

#include "coreSQLiteStudio_global.h"
#include <QString>

class Db;

class API_EXPORT Table
{
    public:
        Table() = default;
        Table(const QString& database, const QString& table);
        Table(const Table& other) = default;
        virtual ~Table() = default;

        bool operator ==(const Table& other) const = default;
        Table& operator=(const Table&) = default;
        Table& operator=(Table&&) = default;

        QString getDatabase() const;
        void setDatabase(const QString& value);

        QString getTable() const;
        void setTable(const QString& value);

    protected:
        QString database;
        QString table;
};

class API_EXPORT DbAndTable : public Table
{
public:
    DbAndTable() = default;
    DbAndTable(Db* db, const QString& database, const QString& table);
    DbAndTable(const DbAndTable& other) = default;

    bool operator ==(const DbAndTable& other) const = default;
    DbAndTable& operator=(const DbAndTable&) = default;
    DbAndTable& operator=(DbAndTable&&) = default;

    Db *getDb() const;
    void setDb(Db *value);

protected:
    Db* db = nullptr;
};

class API_EXPORT AliasedTable : public Table
{
    public:
        AliasedTable() = default;
        AliasedTable(const QString& database, const QString& table, const QString& alias);
        AliasedTable(const AliasedTable& other) = default;
        virtual ~AliasedTable() = default;

        bool operator ==(const AliasedTable& other) const = default;
        AliasedTable& operator=(const AliasedTable&) = default;
        AliasedTable& operator=(AliasedTable&&) = default;

        QString getTableAlias() const;
        void setTableAlias(const QString& value);

    protected:
        QString tableAlias;
};

API_EXPORT TYPE_OF_QHASH qHash(Table table);
API_EXPORT TYPE_OF_QHASH qHash(AliasedTable table);


#endif // TABLE_H
