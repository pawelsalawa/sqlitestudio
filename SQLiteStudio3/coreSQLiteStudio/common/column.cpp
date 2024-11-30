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

TYPE_OF_QHASH qHash(Column column)
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

TYPE_OF_QHASH qHash(AliasedColumn column)
{
    return qHash(column.getDatabase() + "." + column.getTable() + "." + column.getColumn() + "/" + column.getDeclaredType()
                 + "/" + column.getAlias());
}
