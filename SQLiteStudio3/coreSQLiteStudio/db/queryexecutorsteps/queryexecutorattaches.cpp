#include "queryexecutorattaches.h"
#include "dbattacher.h"
#include "sqlitestudio.h"
#include <QScopedPointer>

bool QueryExecutorAttaches::exec()
{
    QScopedPointer<DbAttacher> attacher(SQLITESTUDIO->createDbAttacher(db));
    if (!attacher->attachDatabases(context->parsedQueries))
        return false;

    context->dbNameToAttach = attacher->getDbNameToAttach();
    context->nativeDbPathToAttachName = attacher->getNativePathToAttachName();
    updateQueries();

    return true;
}
