#include "queryexecutorcountresults.h"
#include "parser/ast/sqlitequery.h"
#include "db/queryexecutor.h"
#include <math.h>
#include <QDebug>

bool QueryExecutorCountResults::exec()
{
    SqliteSelectPtr select = getSelect();
    if (!select || select->explain)
    {
        // No return rows, but we're good to go.
        // Pragma and Explain statements use "rows affected" for number of rows.
        return true;
    }

    QString countSql = "SELECT count(*) AS cnt FROM ("+select->detokenize()+");";
    context->countingQuery = countSql;

    // qDebug() << "count sql:" << countSql;
    return true;
}
