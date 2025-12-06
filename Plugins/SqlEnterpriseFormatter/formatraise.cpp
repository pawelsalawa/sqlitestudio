#include "formatraise.h"
#include "parser/ast/sqliteraise.h"
#include "parser/ast/sqliteexpr.h"

FormatRaise::FormatRaise(SqliteRaise *raise) :
    raise(raise)
{
}

void FormatRaise::formatInternal()
{
    withKeyword("RAISE").withParFuncLeft().withKeyword(SqliteRaise::raiseType(raise->type));
    if (raise->type != SqliteRaise::Type::IGNORE)
        withCommaOper().withStatement(raise->expr);

    withParFuncRight();
}
