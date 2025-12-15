#include "sqlitedropview.h"
#include "sqlitequerytype.h"

#include <parser/statementtokenbuilder.h>

SqliteDropView::SqliteDropView()
{
    queryType = SqliteQueryType::DropView;
}

SqliteDropView::SqliteDropView(const SqliteDropView& other) :
    SqliteQuery(other), ifExistsKw(other.ifExistsKw), database(other.database), view(other.view)
{
}

SqliteDropView::SqliteDropView(bool ifExists, const QString &name1, const QString &name2)
    : SqliteDropView()
{
    this->ifExistsKw = ifExists;

    if (name2.isNull())
        view = name1;
    else
    {
        database = name1;
        view = name2;
    }
}

SqliteStatement*SqliteDropView::clone()
{
    return new SqliteDropView(*this);
}

QStringList SqliteDropView::getDatabasesInStatement()
{
    return getStrListFromValue(database);
}

TokenList SqliteDropView::getDatabaseTokensInStatement()
{
    return getDbTokenListFromFullname();
}

QList<SqliteStatement::FullObject> SqliteDropView::getFullObjectsInStatement()
{
    QList<FullObject> result;

    // Table object
    FullObject fullObj = getFullObjectFromFullname(FullObject::VIEW);

    if (fullObj.isValid())
        result << fullObj;

    // Db object
    fullObj = getFirstDbFullObject();
    if (fullObj.isValid())
        result << fullObj;

    return result;
}


TokenList SqliteDropView::rebuildTokensFromContents() const
{
    StatementTokenBuilder builder;
    builder.withTokens(SqliteQuery::rebuildTokensFromContents());
    builder.withKeyword("DROP").withSpace().withKeyword("VIEW").withSpace();

    if (ifExistsKw)
        builder.withKeyword("IF").withSpace().withKeyword("EXISTS").withSpace();

    if (!database.isNull())
        builder.withOther(database).withOperator(".");

    builder.withOther(view).withOperator(";");

    return builder.build();
}
