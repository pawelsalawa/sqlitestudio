#include "formataltertable.h"
#include "parser/ast/sqlitealtertable.h"

FormatAlterTable::FormatAlterTable(SqliteAlterTable* alterTable) :
    FormatStatement(alterTable), alterTable(alterTable)
{
}

void FormatAlterTable::formatInternal()
{
    handleExplainQuery(alterTable);
    withKeyword("ALTER").withKeyword("TABLE");

    if (!alterTable->database.isNull())
        withId(alterTable->database).withIdDot();

    withId(alterTable->table);

    switch (alterTable->command)
    {
        case SqliteAlterTable::Command::SET_NOT_NULL:
            withKeyword("ALTER");
            if (alterTable->columnKw)
                withKeyword("COLUMN");

            withId(alterTable->columnName).withKeyword("SET").withKeyword("NOT").withKeyword("NULL")
                .withConflict(alterTable->onConflict);

            break;
        case SqliteAlterTable::Command::DROP_NOT_NULL:
            withKeyword("ALTER");
            if (alterTable->columnKw)
                withKeyword("COLUMN");

            withId(alterTable->columnName).withKeyword("DROP").withKeyword("NOT").withKeyword("NULL");
            break;
        case SqliteAlterTable::Command::ADD_CHECK:
            withKeyword("ADD");
            if (!alterTable->constraintName.isNull())
                withKeyword("CONSTRAINT").withId(alterTable->constraintName);

            withKeyword("CHECK").withParExprLeft().withStatement(alterTable->expr).withParExprRight()
                .withConflict(alterTable->onConflict);

            break;
        case SqliteAlterTable::Command::DROP_CONSTRAINT:
            withKeyword("DROP").withKeyword("CONSTRAINT").withId(alterTable->constraintName);
            break;
        case SqliteAlterTable::Command::RENAME:
            withKeyword("RENAME").withKeyword("TO").withId(alterTable->newName);
            break;
        case SqliteAlterTable::Command::ADD_COLUMN:
            withKeyword("ADD");
            if (alterTable->columnKw)
                withKeyword("COLUMN");

            withStatement(alterTable->newColumn);
            break;
        case SqliteAlterTable::Command::DROP_COLUMN:
            withKeyword("DROP");
            if (alterTable->columnKw)
                withKeyword("COLUMN");

            withId(alterTable->dropColumnName);
            break;
        case SqliteAlterTable::Command::RENAME_COLUMN:
            withKeyword("RENAME");
            if (alterTable->columnKw)
                withKeyword("COLUMN");

            withId(alterTable->columnName).withKeyword("TO").withId(alterTable->newColumnName);
            break;
        case SqliteAlterTable::Command::null:
            break;
    }
    withSemicolon();
}
