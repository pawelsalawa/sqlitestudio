#include "formatdroptable.h"
#include "parser/ast/sqlitedroptable.h"

FormatDropTable::FormatDropTable(SqliteDropTable* dropTable) :
    dropTable(dropTable)
{
}

void FormatDropTable::formatInternal()
{
    handleExplainQuery(dropTable);
    withKeyword("DROP").withKeyword("TABLE");

    if (dropTable->ifExistsKw)
        withKeyword("IF").withKeyword("EXISTS");

    if (!dropTable->database.isNull())
        withId(dropTable->database).withIdDot();

    withId(dropTable->table).withSemicolon();

}
