#include "formatempty.h"
#include "parser/ast/sqliteemptyquery.h"

FormatEmpty::FormatEmpty(SqliteEmptyQuery* eq) :
    FormatStatement(eq)
{
}

void FormatEmpty::formatInternal()
{
    withSemicolon();
}
