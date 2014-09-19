#include "formatvacuum.h"

FormatVacuum::FormatVacuum(SqliteVacuum* vacuum) :
    vacuum(vacuum)
{
}

void FormatVacuum::formatInternal()
{
    withKeyword("VACUUM").withSemicolon();
}
