#include "formatpragma.h"
#include "parser/ast/sqlitepragma.h"

FormatPragma::FormatPragma(SqlitePragma* pragma) :
    FormatStatement(pragma), pragma(pragma)
{
}

void FormatPragma::formatInternal()
{
    handleExplainQuery(pragma);
    withKeyword("PRAGMA");

    if (!pragma->database.isNull())
        withId(pragma->database).withIdDot();

    withId(pragma->pragmaName);

    if (pragma->equalsOp)
        withOperator("=");
    else if (pragma->parenthesis)
        withParExprLeft();

    if (pragma->value.userType() == QVariant::Bool)
        withId(pragma->getBoolLiteralValue(), false);
    else
        withLiteral(pragma->value);

    if (pragma->parenthesis)
        withParExprRight();

    withSemicolon();
}
