#include "formatpragma.h"
#include "parser/ast/sqlitepragma.h"

FormatPragma::FormatPragma(SqlitePragma* pragma) :
    pragma(pragma)
{
}

void FormatPragma::formatInternal()
{
    withKeyword("PRAGMA");

    if (!pragma->database.isNull())
        withId(pragma->database).withIdDot();

    withId(pragma->pragmaName);

    if (pragma->equalsOp)
        withOperator("=").withLiteral(pragma->value);
    else if (pragma->parenthesis)
        withParExprLeft().withLiteral(pragma->value).withParExprRight();

    withSemicolon();
}
