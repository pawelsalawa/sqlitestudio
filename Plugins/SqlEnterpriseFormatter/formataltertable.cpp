#include "formataltertable.h"
#include "parser/ast/sqlitealtertable.h"

FormatAlterTable::FormatAlterTable(SqliteAlterTable* alterTable) :
    alterTable(alterTable)
{
}

void FormatAlterTable::formatInternal()
{
    handleExplainQuery(alterTable);
    withKeyword("ALTER").withKeyword("TABLE");

    if (!alterTable->database.isNull())
        withId(alterTable->database).withIdDot();

    withId(alterTable->table);

    switch (alterTable->command) {
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

            withId(alterTable->oldColumnName).withKeyword("TO").withId(alterTable->newColumnName);
            break;
        case SqliteAlterTable::Command::null:
            break;
    }
    withSemicolon();
}
