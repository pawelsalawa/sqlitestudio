#include "table.h"
#include <QHash>

Table::Table(const QString& database, const QString& table)
{
    setDatabase(database);
    setTable(table);
}

QString Table::getTable() const
{
    return table;
}

void Table::setTable(const QString& value)
{
    table = value;
}

QString Table::getDatabase() const
{
    return database;
}

void Table::setDatabase(const QString& value)
{
    database = value.isEmpty() ? "main" : value;
}

size_t qHash(Table table)
{
    return qHash(table.getDatabase() + "." + table.getTable());
}

AliasedTable::AliasedTable(const QString& database, const QString& table, const QString& alias) :
    Table(database, table)
{
    setTableAlias(alias);
}

QString AliasedTable::getTableAlias() const
{
    return tableAlias;
}

void AliasedTable::setTableAlias(const QString& value)
{
    tableAlias = value;
}

size_t qHash(AliasedTable table)
{
    return qHash(table.getDatabase() + "." + table.getTable() + " " + table.getTableAlias());
}

DbAndTable::DbAndTable(Db *db, const QString &database, const QString &table) :
    Table(database, table), db(db)
{
}

Db *DbAndTable::getDb() const
{
    return db;
}

void DbAndTable::setDb(Db *value)
{
    db = value;
}
