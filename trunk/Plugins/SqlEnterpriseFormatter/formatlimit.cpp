#include "formatlimit.h"
#include "parser/ast/sqlitelimit.h"
#include "parser/ast/sqliteexpr.h"

FormatLimit::FormatLimit(SqliteLimit *limit) :
    limit(limit)
{
}

void FormatLimit::formatInternal()
{
    if (limit->limit)
        withStatement(limit->limit);

    if (limit->offset)
    {
        if (limit->offsetKw)
            withKeyword("OFFSET");
        else
            withCommaOper();

        withStatement(limit->offset);
    }
}
