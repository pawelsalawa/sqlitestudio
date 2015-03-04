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

    if (alterTable->newColumn)
    {
        withKeyword("ADD");
        if (alterTable->columnKw)
            withKeyword("COLUMN");

        withStatement(alterTable->newColumn);
    }
    else if (!alterTable->newName.isNull())
    {
        withKeyword("RENAME").withKeyword("TO").withId(alterTable->newName);
    }
    withSemicolon();
}
