#include "sqlitewith.h"
#include "parser/statementtokenbuilder.h"
#include "sqliteselect.h"
#include "common/global.h"

SqliteWith::SqliteWith()
{
}

SqliteWith::SqliteWith(const SqliteWith& other) :
    SqliteStatement(other), recursive(other.recursive)
{
    DEEP_COPY_COLLECTION(CommonTableExpression, cteList);
}

SqliteStatement*SqliteWith::clone()
{
    return new SqliteWith(*this);
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

SqliteWith::CommonTableExpression::CommonTableExpression()
{
}

SqliteWith::CommonTableExpression::CommonTableExpression(const SqliteWith::CommonTableExpression& other) :
    SqliteStatement(other), table(other.table), asMode(other.asMode)
{
    DEEP_COPY_COLLECTION(SqliteIndexedColumn, indexedColumns);
    DEEP_COPY_FIELD(SqliteSelect, select);
}

SqliteWith::CommonTableExpression::CommonTableExpression(const QString& tableName, const QList<SqliteIndexedColumn*>& indexedColumns, SqliteSelect* select, AsMode asMode) :
    table(tableName), indexedColumns(indexedColumns), select(select), asMode(asMode)
{
    select->setParent(this);
}

SqliteStatement* SqliteWith::CommonTableExpression::clone()
{
    return new SqliteWith::CommonTableExpression(*this);
}

TokenList SqliteWith::CommonTableExpression::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;
    builder.withOther(table);

    if (indexedColumns.size() > 0)
        builder.withSpace().withParLeft().withStatementList(indexedColumns).withParRight();

    builder.withSpace().withKeyword("AS");
    switch (asMode) {
        case ANY:
            break;
        case MATERIALIZED:
            builder.withKeyword("MATERIALIZED");
            break;
        case NOT_MATERIALIZED:
            builder.withKeyword("NOT").withSpace().withKeyword("MATERIALIZED");
            break;
    }

    builder.withSpace().withParLeft().withStatement(select).withParRight();

    return builder.build();
}
