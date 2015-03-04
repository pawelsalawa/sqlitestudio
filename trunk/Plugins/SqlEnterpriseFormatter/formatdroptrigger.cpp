#include "formatdroptrigger.h"
#include "parser/ast/sqlitedroptrigger.h"

FormatDropTrigger::FormatDropTrigger(SqliteDropTrigger* dropTrig) :
    dropTrig(dropTrig)
{
}

void FormatDropTrigger::formatInternal()
{
    handleExplainQuery(dropTrig);
    withKeyword("DROP").withKeyword("TRIGGER");

    if (dropTrig->ifExistsKw)
        withKeyword("IF").withKeyword("EXISTS");

    if (!dropTrig->database.isNull())
        withId(dropTrig->database).withIdDot();

    withId(dropTrig->trigger).withSemicolon();

}
