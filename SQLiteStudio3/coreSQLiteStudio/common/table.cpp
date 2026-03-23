#include "table.h"
#include "services/dbmanager.h"
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

QDataStream &operator<<(QDataStream &out, const Table& myObj)
{
    out << myObj.getDatabase() << myObj.getTable();
    return out;
}

QDataStream &operator>>(QDataStream &in, Table& myObj)
{
    QString database;
    QString table;
    in >> database >> table;
    myObj.setDatabase(database);
    myObj.setTable(table);
    return in;
}

QDataStream &operator<<(QDataStream &out, const DbAndTable& myObj)
{
    out << myObj.getDatabase() << myObj.getTable();

    if (myObj.getDb())
        out << myObj.getDb()->getName();
    else
        out << QString();

    return out;
}

QDataStream &operator>>(QDataStream &in, DbAndTable& myObj)
{
    QString database;
    QString table;
    QString dbName;

    in >> database >> table >> dbName;
    myObj.setDatabase(database);
    myObj.setTable(table);

    if (!dbName.isEmpty())
        myObj.setDb(DBLIST->getByName(dbName));

    return in;
}

QDataStream &operator<<(QDataStream &out, const AliasedTable& myObj)
{
    out << myObj.getDatabase() << myObj.getTable() << myObj.getTableAlias();
    return out;
}

QDataStream &operator>>(QDataStream &in, AliasedTable& myObj)
{
    QString database;
    QString table;
    QString alias;
    in >> database >> table >> alias;
    myObj.setDatabase(database);
    myObj.setTable(table);
    myObj.setTableAlias(alias);
    return in;
}
