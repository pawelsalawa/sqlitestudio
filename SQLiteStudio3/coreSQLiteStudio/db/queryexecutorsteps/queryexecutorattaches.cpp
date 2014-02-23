#include "queryexecutorattaches.h"
#include "dbattacher.h"

bool QueryExecutorAttaches::exec()
{
    DbAttacher attacher(db);
    if (!attacher.attachDatabases(context->parsedQueries))
        return false;

    context->dbNameToAttach = attacher.getDbNameToAttach();
    updateQueries();

    return true;
}
