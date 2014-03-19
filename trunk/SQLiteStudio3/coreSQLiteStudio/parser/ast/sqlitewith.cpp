#include "sqlitewith.h"
#include "parser/statementtokenbuilder.h"
#include "sqliteselect.h"

SqliteWith::SqliteWith()
{
}

SqliteWith* SqliteWith::append(const QString& tableName, const QList<SqliteIndexedColumn*>& indexedColumns, SqliteSelect* select)
{
    SqliteWith* with = new SqliteWith();
    CommonTableExpression* cte = new CommonTableExpression(tableName, indexedColumns, select);
    cte->setParent(with);
    with->cteList << cte;
    return with;
}

SqliteWith* SqliteWith::append(SqliteWith* with, const QString& tableName, const QList<SqliteIndexedColumn*>& indexedColumns, SqliteSelect* select)
{
    if (!with)
        with = new SqliteWith();

    CommonTableExpression* cte = new CommonTableExpression(tableName, indexedColumns, select);
    cte->setParent(with);
    with->cteList << cte;
    return with;
}

TokenList SqliteWith::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;

    builder.withKeyword("WITH").withSpace();
    if (recursive)
        builder.withKeyword("RECURSIVE").withSpace();

    builder.withStatementList(cteList);

    return builder.build();
}

SqliteWith::CommonTableExpression::CommonTableExpression(const QString& tableName, const QList<SqliteIndexedColumn*>& indexedColumns, SqliteSelect* select) :
    table(tableName), indexedColumns(indexedColumns), select(select)
{
}

TokenList SqliteWith::CommonTableExpression::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;
    builder.withOther(table, dialect).withSpace().withParLeft().withStatementList(indexedColumns).withParRight()
            .withSpace().withKeyword("AS").withSpace().withParLeft().withStatement(select).withParRight();

    return builder.build();
}
