#include "sqliteindexedcolumn.h"
#include "parser/statementtokenbuilder.h"

SqliteIndexedColumn::SqliteIndexedColumn()
{
}

SqliteIndexedColumn::SqliteIndexedColumn(const SqliteIndexedColumn& other) :
    SqliteStatement(other), name(other.name), sortOrder(other.sortOrder), collate(other.collate)
{
}

SqliteIndexedColumn::SqliteIndexedColumn(const QString &name, const QString &collate, SqliteSortOrder sortOrder)
    : SqliteIndexedColumn()
{
    this->name = name;
    this->sortOrder = sortOrder;
    this->collate = collate;
}

SqliteIndexedColumn::SqliteIndexedColumn(const QString& name)
    : SqliteIndexedColumn()
{
    this->name = name;
}

QStringList SqliteIndexedColumn::getColumnsInStatement()
{
    return getStrListFromValue(name);
}

TokenList SqliteIndexedColumn::getColumnTokensInStatement()
{
    return getTokenListFromNamedKey("nm");
}


TokenList SqliteIndexedColumn::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;
    builder.withOther(name, dialect);
    if (!collate.isNull())
        builder.withSpace().withKeyword("COLLATE").withSpace().withOther(collate, dialect);

    builder.withSortOrder(sortOrder);
    return builder.build();
}
