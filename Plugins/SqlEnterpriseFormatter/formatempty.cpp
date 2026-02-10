#include "formatempty.h"

FormatEmpty::FormatEmpty(SqliteEmptyQuery* eq)
{
    Q_UNUSED(eq);
}

void FormatEmpty::formatInternal()
{
    withSemicolon();
}
