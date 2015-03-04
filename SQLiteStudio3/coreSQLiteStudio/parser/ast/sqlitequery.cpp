#include "sqlitequery.h"
#include "parser/statementtokenbuilder.h"

SqliteQuery::SqliteQuery()
{
}

SqliteQuery::SqliteQuery(const SqliteQuery& other) :
    SqliteStatement(other), queryType(other.queryType), explain(other.explain), queryPlan(other.queryPlan)
{
}

bool SqliteQuery::isReadOnly()
{
    bool readOnly = true;
    switch (queryType)
    {
        case SqliteQueryType::EMPTY:
        case SqliteQueryType::Analyze:
        case SqliteQueryType::Pragma:
        case SqliteQueryType::Select:
            readOnly = true;
            break;
        case SqliteQueryType::UNDEFINED:
        case SqliteQueryType::AlterTable:
        case SqliteQueryType::Attach:
        case SqliteQueryType::BeginTrans:
        case SqliteQueryType::CommitTrans:
        case SqliteQueryType::Copy:
        case SqliteQueryType::CreateIndex:
        case SqliteQueryType::CreateTable:
        case SqliteQueryType::CreateTrigger:
        case SqliteQueryType::CreateView:
        case SqliteQueryType::CreateVirtualTable:
        case SqliteQueryType::Delete:
        case SqliteQueryType::Detach:
        case SqliteQueryType::DropIndex:
        case SqliteQueryType::DropTable:
        case SqliteQueryType::DropTrigger:
        case SqliteQueryType::DropView:
        case SqliteQueryType::Insert:
        case SqliteQueryType::Reindex:
        case SqliteQueryType::Release:
        case SqliteQueryType::Rollback:
        case SqliteQueryType::Savepoint:
        case SqliteQueryType::Update:
        case SqliteQueryType::Vacuum:
            readOnly = false;
            break;
    }
    return readOnly;
}

TokenList SqliteQuery::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;
    if (explain)
    {
        builder.withKeyword("EXPLAIN").withSpace();
        if (queryPlan)
            builder.withKeyword("QUERY").withSpace().withKeyword("PLAN").withSpace();
    }
    return builder.build();
}
