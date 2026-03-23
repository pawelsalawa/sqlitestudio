#include "column.h"
#include <QHash>

Column::Column(const QString& database, const QString& table, const QString& column) :
    Table(database, table)
{
    setColumn(column);
}

QString Column::getColumn() const
{
    return column;
}

void Column::setColumn(const QString& value)
{
    column = value;
}

QString Column::getDeclaredType() const
{
    return declaredType;
}

void Column::setDeclaredType(const QString& value)
{
    declaredType = value;
}

size_t qHash(Column column)
{
    return qHash(column.getDatabase() + "." + column.getTable() + "." + column.getColumn() + "/" + column.getDeclaredType());
}

AliasedColumn::AliasedColumn(const QString& database, const QString& table, const QString& column, const QString& alias) :
    Column(database, table, column)
{
    setAlias(alias);
}

QString AliasedColumn::getAlias() const
{
    return alias;
}

void AliasedColumn::setAlias(const QString& value)
{
    alias = value;
}

QDataStream &operator<<(QDataStream &out, const Column& myObj)
{
    out << myObj.getDatabase() << myObj.getTable() << myObj.getColumn() << myObj.getDeclaredType();
    return out;
}

QDataStream &operator>>(QDataStream &in, Column& myObj)
{
    QString database;
    QString table;
    QString column;
    QString declaredType;
    in >> database >> table >> column >> declaredType;
    myObj.setDatabase(database);
    myObj.setTable(table);
    myObj.setColumn(column);
    myObj.setDeclaredType(declaredType);
    return in;
}

QDataStream &operator<<(QDataStream &out, const AliasedColumn& myObj)
{
    out << myObj.getDatabase() << myObj.getTable() << myObj.getColumn() << myObj.getDeclaredType() << myObj.getAlias();
    return out;
}

QDataStream &operator>>(QDataStream &in, AliasedColumn& myObj)
{
    QString database;
    QString table;
    QString column;
    QString declaredType;
    QString alias;
    in >> database >> table >> column >> declaredType >> alias;
    myObj.setDatabase(database);
    myObj.setTable(table);
    myObj.setColumn(column);
    myObj.setDeclaredType(declaredType);
    myObj.setAlias(alias);
    return in;
}

size_t qHash(AliasedColumn column)
{
    return qHash(column.getDatabase() + "." + column.getTable() + "." + column.getColumn() + "/" + column.getDeclaredType()
                 + "/" + column.getAlias());
}
