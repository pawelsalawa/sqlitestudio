#include "sqlitepragma.h"
#include "sqlitequerytype.h"

#include <parser/statementtokenbuilder.h>

SqlitePragma::SqlitePragma()
{
    queryType = SqliteQueryType::Pragma;
}

SqlitePragma::SqlitePragma(const SqlitePragma& other) :
    SqliteQuery(other), database(other.database), pragmaName(other.pragmaName), value(other.value), equalsOp(other.equalsOp), parenthesis(other.parenthesis)
{
}

SqlitePragma::SqlitePragma(const QString &name1, const QString &name2)
    : SqlitePragma()
{
    initName(name1, name2);
}

SqlitePragma::SqlitePragma(const QString &name1, const QString &name2, const QVariant& value, bool equals)
    : SqlitePragma()
{
    initName(name1, name2);
    if (!handleBoolValue(value))
        this->value = value;

    if (equals)
        equalsOp = true;
    else
        parenthesis = true;
}

SqlitePragma::SqlitePragma(const QString &name1, const QString &name2, const QString &value, bool equals)
    : SqlitePragma()
{
    initName(name1, name2);
    if (!handleBoolValue(value))
        this->value = value;

    if (equals)
        equalsOp = true;
    else
        parenthesis = true;
}

SqliteStatement*SqlitePragma::clone()
{
    return new SqlitePragma(*this);
}

QString SqlitePragma::getBoolLiteralValue() const
{
    if (onOffKw)
        return value.toBool() ? "on" : "off";
    else if (yesNoKw)
        return value.toBool() ? "yes" : "no";
    else
        return value.toBool() ? "true" : "false";
}

QStringList SqlitePragma::getDatabasesInStatement()
{
    return getStrListFromValue(database);
}

TokenList SqlitePragma::getDatabaseTokensInStatement()
{
    if (database.isNull())
        return TokenList();

    return getTokenListFromNamedKey("nm");
}

QList<SqliteStatement::FullObject> SqlitePragma::getFullObjectsInStatement()
{
    QList<FullObject> result;
    if (database.isNull())
        return result;

    // Db object
    FullObject fullObj = getFirstDbFullObject();
    if (fullObj.isValid())
    {
        result << fullObj;
        dbTokenForFullObjects = fullObj.database;
    }

    return result;
}

void SqlitePragma::initName(const QString &name1, const QString &name2)
{
    if (!name2.isNull())
    {
        database = name1;
        pragmaName = name2;
    }
    else
        pragmaName = name1;
}

bool SqlitePragma::handleBoolValue(const QVariant &value)
{
    QString lower = value.toString().toLower();
    if (lower == "true")
        this->value = true;
    else if (lower == "false")
        this->value = false;
    else if (lower == "on")
    {
        this->value = true;
        this->onOffKw = true;
    }
    else if (lower == "off")
    {
        this->value = false;
        this->onOffKw = true;
    }
    else if (lower == "yes")
    {
        this->value = true;
        this->yesNoKw = true;
    }
    else if (lower == "no")
    {
        this->value = false;
        this->yesNoKw = true;
    }
    else
        return false;

    return true;
}

TokenList SqlitePragma::rebuildTokensFromContents(bool replaceStatementTokens) const
{
    StatementTokenBuilder builder(replaceStatementTokens);
    builder.withTokens(SqliteQuery::rebuildTokensFromContents(replaceStatementTokens));
    builder.withKeyword("PRAGMA").withSpace();

    if (!database.isNull())
        builder.withOther(database).withOperator(".");

    builder.withOther(pragmaName);

    if (equalsOp)
        builder.withSpace().withOperator("=").withSpace();
    else if (parenthesis)
        builder.withParLeft();

    if (value.userType() == QVariant::Bool)
        builder.withOther(getBoolLiteralValue(), false);
    else
        builder.withLiteralValue(value);

    if (parenthesis)
        builder.withParRight();

    builder.withOperator(";");

    return builder.build();
}
