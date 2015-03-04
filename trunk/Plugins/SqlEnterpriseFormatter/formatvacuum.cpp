#include "formatvacuum.h"
#include "parser/ast/sqlitevacuum.h"

FormatVacuum::FormatVacuum(SqliteVacuum* vacuum) :
    vacuum(vacuum)
{
}

void FormatVacuum::formatInternal()
{
    handleExplainQuery(vacuum);
    withKeyword("VACUUM").withSemicolon();
}
