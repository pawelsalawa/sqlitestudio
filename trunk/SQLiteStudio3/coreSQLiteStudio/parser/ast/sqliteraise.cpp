#include "sqliteraise.h"
#include "parser/statementtokenbuilder.h"

SqliteRaise::SqliteRaise()
{
}

SqliteRaise::SqliteRaise(const SqliteRaise& other) :
    SqliteStatement(other), type(other.type), message(other.message)
{
}

SqliteRaise::SqliteRaise(const QString &type)
{
    this->type = raiseType(type);
}

SqliteRaise::SqliteRaise(const QString &type, const QString &text)
{
    this->type = raiseType(type);
    message = text;
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
            return QString::null;
    }
}

TokenList SqliteRaise::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;
    builder.withKeyword("RAISE").withSpace().withParLeft().withKeyword(raiseType(type));
    if (type != Type::IGNORE)
        builder.withOperator(",").withSpace().withString(message);

    builder.withParRight();
    return builder.build();
}
