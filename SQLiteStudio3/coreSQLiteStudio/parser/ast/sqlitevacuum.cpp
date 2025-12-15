#include "sqlitevacuum.h"
#include "sqlitequerytype.h"
#include "common/global.h"
#include "parser/statementtokenbuilder.h"
#include "sqliteexpr.h"

SqliteVacuum::SqliteVacuum()
{
    queryType = SqliteQueryType::Vacuum;
}

SqliteVacuum::SqliteVacuum(const SqliteVacuum& other) :
    SqliteQuery(other), database(other.database)
{
    DEEP_COPY_FIELD(SqliteExpr, expr);
}

SqliteVacuum::SqliteVacuum(SqliteExpr* expr)
    : SqliteVacuum()
{
    this->expr = expr;
    if (expr)
        expr->setParent(this);
}

SqliteVacuum::SqliteVacuum(const QString& name, SqliteExpr* expr)
    : SqliteVacuum()
{
    if (!name.isNull())
        database = name;

    this->expr = expr;
    if (expr)
        expr->setParent(this);
}

SqliteStatement*SqliteVacuum::clone()
{
    return new SqliteVacuum(*this);
}

QStringList SqliteVacuum::getDatabasesInStatement()
{
    return getStrListFromValue(database);
}

TokenList SqliteVacuum::getDatabaseTokensInStatement()
{
    if (!tokensMap.contains("nm"))
        return TokenList();

    return getTokenListFromNamedKey("nm");
}

QList<SqliteStatement::FullObject> SqliteVacuum::getFullObjectsInStatement()
{
    QList<FullObject> result;

    // Db object
    FullObject fullObj = getFirstDbFullObject();
    if (fullObj.isValid())
        result << fullObj;

    return result;
}


TokenList SqliteVacuum::rebuildTokensFromContents() const
{
    StatementTokenBuilder builder;
    builder.withTokens(SqliteQuery::rebuildTokensFromContents());
    builder.withKeyword("VACUUM");
    if (!database.isNull())
        builder.withSpace().withOther(database);

    if (expr)
        builder.withSpace().withKeyword("INTO").withSpace().withStatement(expr);

    builder.withOperator(";");
    return builder.build();
}
