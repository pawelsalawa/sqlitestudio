#include "queryexecutorsqlite2delete.h"
#include "db/sqlquery.h"
#include "parser/ast/sqlitedelete.h"

bool QueryExecutorSqlite2Delete::exec()
{
    if (db->getVersion() != 2)
        return true;

    SqliteQueryPtr lastQuery = context->parsedQueries.last();

    if (!lastQuery)
        return true;

    SqliteDeletePtr deleteQuery = lastQuery.dynamicCast<SqliteDelete>();
    if (!deleteQuery)
        return true;

    if (!deleteQuery->where)
    {
        deleteQuery->where = new SqliteExpr();
        deleteQuery->where->setParent(deleteQuery.data());
        deleteQuery->where->mode = SqliteExpr::Mode::LITERAL_VALUE;
        deleteQuery->where->literalValue = 1;
    }

    deleteQuery->rebuildTokens();
    updateQueries();
    return true;
}
