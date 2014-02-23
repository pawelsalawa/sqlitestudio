#include "sqlitepragma.h"
#include "sqlitequerytype.h"

SqlitePragma::SqlitePragma()
{
    queryType = SqliteQueryType::Pragma;
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
    this->value = value;
    if (equals)
        equalsOp = true;
    else
        parenthesis = true;
}

QStringList SqlitePragma::getDatabasesInStatement()
{
    return getStrListFromValue(database);
}

TokenList SqlitePragma::getDatabaseTokensInStatement()
{
    if (dialect == Dialect::Sqlite2 || database.isNull())
        return TokenList();

    return getTokenListFromNamedKey("nm");
}

QList<SqliteStatement::FullObject> SqlitePragma::getFullObjectsInStatement()
{
    QList<FullObject> result;
    if (dialect == Dialect::Sqlite2 || database.isNull())
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
