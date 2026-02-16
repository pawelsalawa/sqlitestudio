#include "formatvacuum.h"
#include "parser/ast/sqlitevacuum.h"
#include "parser/ast/sqliteexpr.h"

FormatVacuum::FormatVacuum(SqliteVacuum* vacuum) :
    FormatStatement(vacuum), vacuum(vacuum)
{
}

void FormatVacuum::formatInternal()
{
    handleExplainQuery(vacuum);
    withKeyword("VACUUM").withSemicolon();
    if (!vacuum->database.isNull())
        withId(vacuum->database);

    if (vacuum->expr)
        withKeyword("INTO").withStatement(vacuum->expr);
}
