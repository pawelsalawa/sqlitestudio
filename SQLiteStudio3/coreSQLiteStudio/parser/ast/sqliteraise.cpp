#include "sqliteraise.h"
#include "parser/statementtokenbuilder.h"
#include "common/global.h"
#include "sqliteexpr.h"

SqliteRaise::SqliteRaise()
{
}

SqliteRaise::SqliteRaise(const SqliteRaise& other) :
    SqliteStatement(other), type(other.type)
{
    DEEP_COPY_FIELD(SqliteExpr, expr);
}

SqliteRaise::SqliteRaise(const QString &type)
{
    this->type = raiseType(type);
}

SqliteRaise::SqliteRaise(const QString &type, SqliteExpr *value)
{
    this->type = raiseType(type);
    this->expr = value;
}

SqliteStatement*SqliteRaise::clone()
{
    return new SqliteRaise(*this);
}

SqliteRaise::Type SqliteRaise::raiseType(const QString &value)
{
    QString upper = value.toUpper();
    if (upper == "IGNORE")
        return SqliteRaise::Type::IGNORE;
    else if (upper == "ROLLBACK")
        return SqliteRaise::Type::ROLLBACK;
    else if (upper == "ABORT")
        return SqliteRaise::Type::ABORT;
    else if (upper == "FAIL")
        return SqliteRaise::Type::FAIL;
    else
        return SqliteRaise::Type::null;
}

QString SqliteRaise::raiseType(SqliteRaise::Type value)
{
    switch (value)
    {
        case SqliteRaise::Type::IGNORE:
            return "IGNORE";
        case SqliteRaise::Type::ROLLBACK:
            return "ROLLBACK";
        case SqliteRaise::Type::ABORT:
            return "ABORT";
        case SqliteRaise::Type::FAIL:
            return "FAIL";
        default:
            return QString();
    }
}

TokenList SqliteRaise::rebuildTokensFromContents() const
{
    StatementTokenBuilder builder;
    builder.withKeyword("RAISE").withSpace().withParLeft().withKeyword(raiseType(type));
    if (type != Type::IGNORE)
        builder.withOperator(",").withSpace().withStatement(expr);

    builder.withParRight();
    return builder.build();
}
