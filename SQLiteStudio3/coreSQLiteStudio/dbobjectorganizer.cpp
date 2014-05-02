#include "dbobjectorganizer.h"
#include "schemaresolver.h"
#include "db/db.h"
#include "common/utils_sql.h"
#include "services/notifymanager.h"
#include "db/attachguard.h"
#include <QDebug>
#include <QThreadPool>

DbObjectOrganizer::DbObjectOrganizer()
{
    // Default organizaer denies any referenced objects
    confirmFunction = [](const QStringList&) -> bool {return false;};
    nameConflictResolveFunction = [](QString&) -> bool {return false;};
}

DbObjectOrganizer::DbObjectOrganizer(DbObjectOrganizer::ReferencedTablesConfimFunction confirmFunction,
                                     NameConflictResolveFunction nameConflictResolveFunction) :
    confirmFunction(confirmFunction), nameConflictResolveFunction(nameConflictResolveFunction)
{
}

DbObjectOrganizer::~DbObjectOrganizer()
{
    safe_delete(srcResolver);
    safe_delete(dstResolver);
}

void DbObjectOrganizer::copyObjectsToDb(Db* srcDb, const QStringList& objNames, Db* dstDb, bool includeData)
{
    copyOrMoveObjectsToDb(srcDb, objNames, dstDb, includeData, false);
}

void DbObjectOrganizer::moveObjectsToDb(Db* srcDb, const QStringList& objNames, Db* dstDb, bool includeData)
{
    copyOrMoveObjectsToDb(srcDb, objNames, dstDb, includeData, true);
}

void DbObjectOrganizer::interrupt()
{
    QMutexLocker locker(&interruptMutex);
    interrupted = true;
    srcDb->interrupt();
    dstDb->interrupt();
}

bool DbObjectOrganizer::isExecuting()
{
    QMutexLocker lock(&executingMutex);
    return executing;
}

void DbObjectOrganizer::run()
{
    if (!srcDb->isOpen())
    {
        notifyError(tr("Cannot copy or move objects from closed database. Open it first."));
        emitFinished(false);
        return;
    }

    if (!dstDb->isOpen())
    {
        notifyError(tr("Cannot copy or move objects to closed database. Open it first."));
        emitFinished(false);
        return;
    }

    bool res = processAll();
    emitFinished(res);
}

void DbObjectOrganizer::reset()
{
    mode = Mode::COPY_OBJECTS;
    srcDb = nullptr;
    dstDb = nullptr;
    srcNames.clear();
    srcTables.clear();
    srcViews.clear();
    renamed.clear();
    srcTable = QString::null;
    includeData = false;
    deleteSourceObjects = false;
    referencedTables.clear();
    safe_delete(srcResolver);
    safe_delete(dstResolver);
    interrupted = false;
    setExecuting(false);
}

void DbObjectOrganizer::copyOrMoveObjectsToDb(Db* srcDb, const QStringList& objNames, Db* dstDb, bool includeData, bool move)
{
    if (isExecuting())
    {
        notifyError("Schema modification is currently in progress. Please try again in a moment.");
        qWarning() << "Tried to call DbObjectOrganizer::copyOrMoveObjectsToDb() while other execution was in progress.";
        return;
    }

    reset();
    setExecuting(true);
    if (move)
    {
        mode = Mode::MOVE_OBJECTS;
        deleteSourceObjects = true;
    }
    else
    {
        mode = Mode::COPY_OBJECTS;
    }

    this->srcNames = objNames;
    this->includeData = includeData;
    setSrcAndDstDb(srcDb, dstDb);

    QHash<QString, SchemaResolver::ObjectDetails> details = srcResolver->getAllObjectDetails();
    for (const QString& srcName : srcNames)
    {
        if (!details.contains(srcName))
        {
            qDebug() << "Object" << srcName << "not found in source database, skipping.";
            continue;
        }

        switch (details[srcName].type)
        {
            case SchemaResolver::ObjectDetails::TABLE:
                srcTables << srcName;
                break;
            case SchemaResolver::ObjectDetails::INDEX:
                break;
            case SchemaResolver::ObjectDetails::TRIGGER:
                break;
            case SchemaResolver::ObjectDetails::VIEW:
                srcViews << srcName;
                break;
        }
    }

    for (const QString& table : srcTables)
        collectReferencedTables(table);

    if (referencedTables.size() > 0 && !confirmFunction(referencedTables))
        referencedTables.clear();

    if (!resolveNameConflicts())
    {
        setExecuting(false);
        return;
    }

    QThreadPool::globalInstance()->start(this);
}

bool DbObjectOrganizer::processAll()
{
    dstDb->begin();
    if (!setFkEnabled(false))
    {
        dstDb->rollback();
        return false;
    }

    bool res = false;
    switch (mode)
    {
        case Mode::COPY_OBJECTS:
        case Mode::MOVE_OBJECTS:
        {
            res = processDbObjects();
            break;
        }
        case Mode::unknown:
        {
            qWarning() << "Unhandled unknown mode in DbObjectOrganizer.";
            return false;
        }
    }

    if (!res)
    {
        dstDb->rollback();
        setFkEnabled(true);
        return false;
    }

    if (!setFkEnabled(true))
    {
        dstDb->rollback();
        return false;
    }

    if (!dstDb->commit())
    {
        notifyError(tr("Could not commit transaction in database '%1'.").arg(dstDb->getName()));
        dstDb->rollback();
    }

    return true;
}

bool DbObjectOrganizer::processDbObjects()
{
    for (const QString& table : (referencedTables + srcTables))
    {
        if (!copyTableToDb(table) || isInterrupted())
            return false;
    }

    for (const QString& view : srcViews)
    {
        if (!copyViewToDb(view) || isInterrupted())
            return false;
    }

    if (deleteSourceObjects)
    {
        for (const QString& table : (referencedTables + srcTables))
            dropTable(table);

        for (const QString& view : srcViews)
            dropView(view);
    }

    return true;
}

bool DbObjectOrganizer::resolveNameConflicts()
{
    QStringList names;
    QStringList namesInDst;
    switch (mode)
    {
        case Mode::COPY_OBJECTS:
        case Mode::MOVE_OBJECTS:
        {
            names = referencedTables + srcTables + srcViews;
            namesInDst = dstResolver->getAllObjects();
            break;
        }
        case Mode::unknown:
        {
            qWarning() << "Unhandled unknown mode in DbObjectOrganizer.";
            return false;
        }
    }

    QString finalName;
    for (const QString& srcName : names)
    {
        finalName = srcName;
        while (namesInDst.contains(finalName, Qt::CaseInsensitive))
        {
            if (!nameConflictResolveFunction(finalName))
                return false;
        }
        if (finalName != srcName)
            renamed[srcName] = finalName;
    }
    return true;
}

bool DbObjectOrganizer::copyTableToDb(const QString& table)
{
    QString ddl;
    QString targetTable = table;
    AttachGuard attach = srcDb->guardedAttach(dstDb, true);
    QString attachName = attach->getName();
    if (renamed.contains(table) || !attachName.isNull())
    {
        SqliteQueryPtr parsedObject = srcResolver->getParsedObject(table);
        SqliteCreateTablePtr createTable = parsedObject.dynamicCast<SqliteCreateTable>();
        if (!createTable)
        {
            qCritical() << "Could not parse table while copying:" << table << ", ddl:" << srcResolver->getObjectDdl(table);
            notifyError(tr("Error while creating table in target database: %1").arg(tr("Could not parse table.")));
            return false;
        }

        if (renamed.contains(table))
            targetTable = renamed[table];

        createTable->table = targetTable;
        if (!attachName.isNull())
            createTable->database = attachName;

        createTable->rebuildTokens();
        ddl = createTable->detokenize();
    }
    else
    {
        ddl = srcResolver->getObjectDdl(table);
    }

    SqlResultsPtr result;

    if (attachName.isNull())
        result = dstDb->exec(ddl);
    else
        result = srcDb->exec(ddl); // uses attachName to create object in attached db

    if (result->isError())
    {
        notifyError(tr("Error while creating table in target database: %1").arg(result->getErrorText()));
        return false;
    }

    if (!includeData)
        return true;

    if (isInterrupted())
        return false;

    srcTable = table;
    bool res;
    if (attachName.isNull())
    {
        notifyInfo(tr("Database %1 could not be attached to database %2, so the data of table %3 will be copied "
                      "with SQLiteStudio as a mediator. This method can be slow for huge tables, so please be patient.")
                   .arg(dstDb->getName(), srcDb->getName(), srcTable));

        res = copyDataAsMiddleware(targetTable);
    }
    else
    {
        res = copyDataUsingAttach(targetTable, attachName);
    }
    return res;
}

bool DbObjectOrganizer::copyDataAsMiddleware(const QString& table)
{
    QString wrappedSrcTable = wrapObjIfNeeded(srcTable, srcDb->getDialect());
    SqlResultsPtr results = srcDb->exec("SELECT * FROM "+wrappedSrcTable);
    if (results->isError())
    {
        notifyError(tr("Error while copying data for table %1: %2").arg(table).arg(results->getErrorText()));
        return false;
    }

    QStringList argPlaceholderList;
    for (int i = 0; i < results->getColumnNames().size(); i++)
        argPlaceholderList << "?";

    QString wrappedDstTable = wrapObjIfNeeded(table, dstDb->getDialect());
    QString sql = "INSERT INTO " + wrappedDstTable + " VALUES (" + argPlaceholderList.join(", ") + ")";

    SqlResultsPtr insertResults;
    SqlResultsRowPtr row;
    int i = 0;
    while (results->hasNext())
    {
        row = results->next();
        if (!row)
        {
            notifyError(tr("Error while copying data to table %1: %2").arg(table).arg(results->getErrorText()));
            return false;
        }

        // TODO cases like this require API for re-executing same SQL faster, without compiling query each time
        insertResults = dstDb->exec(sql, row->valueList());
        if (insertResults->isError())
        {
            notifyError(tr("Error while copying data to table %1: %2").arg(table).arg(insertResults->getErrorText()));
            return false;
        }

        if ((i % 1000) == 0 && isInterrupted())
            return false;

        i++;
    }

    if (isInterrupted())
        return false;

    return true;
}

bool DbObjectOrganizer::copyDataUsingAttach(const QString& table, const QString& attachName)
{
    QString wrappedSrcTable = wrapObjIfNeeded(srcTable, srcDb->getDialect());
    QString wrappedDstTable = wrapObjIfNeeded(table, srcDb->getDialect());
    SqlResultsPtr results = srcDb->exec("INSERT INTO " + attachName + "." + wrappedDstTable + " SELECT * FROM " + wrappedSrcTable);
    if (results->isError())
    {
        notifyError(tr("Error while copying data to table %1: %2").arg(table).arg(results->getErrorText()));
        return false;
    }
    return true;
}

void DbObjectOrganizer::dropTable(const QString& table)
{
    QString wrappedSrcTable = wrapObjIfNeeded(table, srcDb->getDialect());
    SqlResultsPtr results = srcDb->exec("DROP TABLE " + wrappedSrcTable);
    if (results->isError())
    {
        notifyWarn(tr("Error while dropping source table %1: %2\nTables and Views copied to database %3 will remain.")
                   .arg(table).arg(results->getErrorText()).arg(dstDb->getName()));
    }
}

void DbObjectOrganizer::dropView(const QString& view)
{
    QString wrappedSrcView = wrapObjIfNeeded(view, srcDb->getDialect());
    SqlResultsPtr results = srcDb->exec("DROP VIEW " + wrappedSrcView);
    if (results->isError())
    {
        notifyWarn(tr("Error while dropping source view %1: %2\nTables and Views copied to database %3 will remain.")
                   .arg(view).arg(results->getErrorText()).arg(dstDb->getName()));
    }
}

bool DbObjectOrganizer::copyViewToDb(const QString& view)
{
    QString ddl = srcResolver->getObjectDdl(view);
    SqlResultsPtr result = dstDb->exec(ddl);
    if (result->isError())
    {
        notifyError(tr("Error while creating view in target database: %1").arg(result->getErrorText()));
        return false;
    }

    return true;
}

QStringList DbObjectOrganizer::resolveReferencedtables(const QString& table)
{
    SchemaResolver resolver(srcDb);
    QStringList tables = resolver.getFkReferencingTables(table);
    QStringList subTables;
    for (const QString& fkTable : tables)
    {
        subTables = resolver.getFkReferencingTables(fkTable);
        for (const QString& subTable : subTables)
        {
            if (tables.contains(subTable))
                continue;

            tables.prepend(subTable);
        }
    }
    return tables;
}

void DbObjectOrganizer::collectReferencedTables(const QString& table)
{
    QStringList tables = resolveReferencedtables(table);
    for (const QString& refTable : tables)
    {
        if (!referencedTables.contains(refTable) && !srcTables.contains(refTable))
            referencedTables << refTable;
    }
}

bool DbObjectOrganizer::setFkEnabled(bool enabled)
{
    if (dstDb->getVersion() == 2)
        return true;

    SqlResultsPtr result = dstDb->exec(QString("PRAGMA foreign_keys = %1").arg(enabled ? "on" : "off"));
    if (result->isError())
    {
        notifyError(tr("Error while executing PRAGMA on target database: %1").arg(result->getErrorText()));
        return false;
    }
    return true;
}

bool DbObjectOrganizer::isInterrupted()
{
    QMutexLocker locker(&interruptMutex);
    return interrupted;
}

void DbObjectOrganizer::setExecuting(bool executing)
{
    QMutexLocker lock(&executingMutex);
    this->executing = executing;
}

void DbObjectOrganizer::setSrcAndDstDb(Db* srcDb, Db* dstDb)
{
    safe_delete(srcResolver);
    safe_delete(dstResolver);
    this->srcDb = srcDb;
    this->dstDb = dstDb;
    srcResolver = new SchemaResolver(srcDb);
    dstResolver = new SchemaResolver(dstDb);
}

void DbObjectOrganizer::emitFinished(bool success)
{
    switch (mode)
    {
        case Mode::COPY_OBJECTS:
            emit finishedDbObjectsCopy(success, srcDb, dstDb);
            break;
        case Mode::MOVE_OBJECTS:
            emit finishedDbObjectsMove(success, srcDb, dstDb);
            break;
        case Mode::unknown:
            break;
    }
    setExecuting(false);
}
