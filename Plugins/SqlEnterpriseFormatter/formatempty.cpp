#include "formatempty.h"
#include "common/unused.h"

FormatEmpty::FormatEmpty(SqliteEmptyQuery* eq)
{
    UNUSED(eq);
}

void FormatEmpty::formatInternal()
{
    withSemicolon();
}
