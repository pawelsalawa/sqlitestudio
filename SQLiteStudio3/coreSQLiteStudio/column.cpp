#include "column.h"
#include <QHash>

Column::Column() : Table()
{
}

Column::Column(const QString& database, const QString& table, const QString& column) :
    Table(database, table)
{
    setColumn(column);
}

Column::Column(const Column& other) :
    Table(other.database, other.table)
{
    column = other.column;
}

int Column::operator ==(const Column& other) const
{
    return Table::operator==(other) && column == other.column;
}

QString Column::getColumn() const
{
    return column;
}

void Column::setColumn(const QString& value)
{
    column = value;
}

int qHash(Column column)
{
    return qHash(column.getDatabase() + "." + column.getTable() + "." + column.getColumn());
}
