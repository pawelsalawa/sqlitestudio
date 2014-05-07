#include "dbobjectorganizer.h"
#include "db/db.h"
#include "common/utils_sql.h"
#include "services/notifymanager.h"
#include "db/attachguard.h"
#include "dbversionconverter.h"
#include <QDebug>
#include <QThreadPool>

DbObjectOrganizer::DbObjectOrganizer()
{
    // Default organizaer denies any referenced objects
    confirmFunction = [](const QStringList&) -> bool {return false;};
    nameConflictResolveFunction = [](QString&) -> bool {return false;};
    conversionConfimFunction = [](const QList<QPair<QString,QString>>&) -> bool {return false;};
    conversionErrorsConfimFunction = [](const QHash<QString,QStringList>&) -> bool {return false;};
    init();
}

DbObjectOrganizer::DbObjectOrganizer(DbObjectOrganizer::ReferencedTablesConfimFunction confirmFunction, NameConflictResolveFunction nameConflictResolveFunction,
                                     ConversionConfimFunction conversionConfimFunction, ConversionErrorsConfimFunction conversionErrorsConfimFunction) :
    confirmFunction(confirmFunction), nameConflictResolveFunction(nameConflictResolveFunction), conversionConfimFunction(conversionConfimFunction),
    conversionErrorsConfimFunction(conversionErrorsConfimFunction)
{
    init();
}

DbObjectOrganizer::~DbObjectOrganizer()
{
    safe_delete(srcResolver);
    safe_delete(dstResolver);
    safe_delete(versionConverter);
}

void DbObjectOrganizer::init()
{
    versionConverter = new DbVersionConverter();
    connect(this, SIGNAL(preparetionFinished()), this, SLOT(processPreparationFinished()));
}

void DbObjectOrganizer::copyObjectsToDb(Db* srcDb, const QStringList& objNames, Db* dstDb, bool includeData, bool includeIndexes, bool includeTriggers)
{
    copyOrMoveObjectsToDb(srcDb, objNames.toSet(), dstDb, includeData, includeIndexes, includeTriggers, false);
}

void DbObjectOrganizer::moveObjectsToDb(Db* srcDb, const QStringList& objNames, Db* dstDb, bool includeData, bool includeIndexes, bool includeTriggers)
{
    copyOrMoveObjectsToDb(srcDb, objNames.toSet(), dstDb, includeData, includeIndexes, includeTriggers, true);
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
    switch (mode)
    {
        case Mode::PREPARE_TO_COPY_OBJECTS:
        case Mode::PREPARE_TO_MOVE_OBJECTS:
            processPreparation();
            break;
        case Mode::COPY_OBJECTS:
        case Mode::MOVE_OBJECTS:
            emitFinished(processAll());
            break;
        case Mode::unknown:
            qCritical() << "DbObjectOrganizer::run() called with unknown mode.";
            emitFinished(false);
            return;
    }
}

void DbObjectOrganizer::reset()
{
    mode = Mode::COPY_OBJECTS;
    srcDb = nullptr;
    dstDb = nullptr;
    srcNames.clear();
    srcTables.clear();
    srcIndexes.clear();
    srcTriggers.clear();
    srcViews.clear();
    renamed.clear();
    srcTable = QString::null;
    includeData = false;
    includeIndexes = false;
    includeTriggers = false;
    deleteSourceObjects = false;
    referencedTables.clear();
    diffListToConfirm.clear();
    errorsToConfirm.clear();
    safe_delete(srcResolver);
    safe_delete(dstResolver);
    interrupted = false;
    setExecuting(false);
}

void DbObjectOrganizer::copyOrMoveObjectsToDb(Db* srcDb, const QSet<QString>& objNames, Db* dstDb, bool includeData, bool includeIndexes, bool includeTriggers, bool move)
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
        mode = Mode::PREPARE_TO_MOVE_OBJECTS;
        deleteSourceObjects = true;
    }
    else
    {
        mode = Mode::PREPARE_TO_COPY_OBJECTS;
    }

    this->srcNames = objNames;
    this->includeData = includeData;
    this->includeIndexes = includeIndexes;
    this->includeTriggers = includeTriggers;
    setSrcAndDstDb(srcDb, dstDb);

    QThreadPool::globalInstance()->start(this);
}

void DbObjectOrganizer::processPreparation()
{
    QHash<QString, SqliteQueryPtr> allParsedObjects = srcResolver->getAllParsedObjects();
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
                collectReferencedTables(srcName, allParsedObjects);
                collectReferencedIndexes(srcName);
                collectReferencedTriggersForTable(srcName);
                break;
            case SchemaResolver::ObjectDetails::INDEX:
                break;
            case SchemaResolver::ObjectDetails::TRIGGER:
                break;
            case SchemaResolver::ObjectDetails::VIEW:
                srcViews << srcName;
                collectReferencedTriggersForView(srcName);
                break;
        }
    }

    if (referencedTables.size() > 0 && !confirmFunction(referencedTables.toList()))
        referencedTables.clear();

    for (const QString& srcTable : referencedTables)
    {
        collectReferencedIndexes(srcTable);
        collectReferencedTriggersForTable(srcTable);
    }

    collectDiffs(details);

    emit preparetionFinished();
}

bool DbObjectOrganizer::processAll()
{
    if (!srcDb->isOpen())
    {
        //notifyError(tr("Cannot copy or move objects from closed database. Open it first.")); // TODO this is in another thread - handle it
        return false;
    }

    if (!dstDb->isOpen())
    {
        //notifyError(tr("Cannot copy or move objects to closed database. Open it first.")); // TODO this is in another thread - handle it
        return false;
    }

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
        case Mode::PREPARE_TO_COPY_OBJECTS:
        case Mode::PREPARE_TO_MOVE_OBJECTS:
        {
            qCritical() << "DbObjectOrganizer::processAll() called with PREAPRE mode.";
            return false; // this method should not be called with this mode
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

    if (includeIndexes)
    {
        for (const QString& idx : srcIndexes)
        {
            if (!copyIndexToDb(idx) || isInterrupted())
                return false;
        }
    }

    if (includeTriggers)
    {
        for (const QString& trig : srcTriggers)
        {
            if (!copyTriggerToDb(trig) || isInterrupted())
                return false;
        }
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
    QSet<QString> names;
    QStringList namesInDst;
    switch (mode)
    {
        case Mode::PREPARE_TO_COPY_OBJECTS:
        case Mode::PREPARE_TO_MOVE_OBJECTS:
        case Mode::COPY_OBJECTS:
        case Mode::MOVE_OBJECTS:
        {
            names = referencedTables + srcTables + srcViews + srcIndexes + srcTriggers;
            namesInDst = dstResolver->getAllObjects();
            break;
        }
        case Mode::unknown:
        {
            qWarning() << "Unhandled unknown mode in DbObjectOrganizer::resolveNameConflicts().";
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

    ddl = convertDdlToDstVersion(ddl);
    if (ddl.trimmed() == ";") // empty query, result of ignored errors in UI
        return true;

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
    dropObject(table, "TABLE");
}

void DbObjectOrganizer::dropView(const QString& view)
{
    dropObject(view, "VIEW");
}

void DbObjectOrganizer::dropObject(const QString& name, const QString& type)
{
    QString wrappedSrcObj = wrapObjIfNeeded(name, srcDb->getDialect());
    SqlResultsPtr results = srcDb->exec("DROP " + type + " " + wrappedSrcObj);
    if (results->isError())
    {
        notifyWarn(tr("Error while dropping source view %1: %2\nTables, indexes, triggers and views copied to database %3 will remain.")
                   .arg(name).arg(results->getErrorText()).arg(dstDb->getName()));
    }
}

bool DbObjectOrganizer::copyViewToDb(const QString& view)
{
    return copySimpleObjectToDb(view, tr("Error while creating view in target database: %1"));
}

bool DbObjectOrganizer::copyIndexToDb(const QString& index)
{
    return copySimpleObjectToDb(index, tr("Error while creating index in target database: %1"));
}

bool DbObjectOrganizer::copyTriggerToDb(const QString& trigger)
{
    return copySimpleObjectToDb(trigger, tr("Error while creating trigger in target database: %1"));
}

bool DbObjectOrganizer::copySimpleObjectToDb(const QString& name, const QString& errorMessage)
{
    QString ddl = srcResolver->getObjectDdl(name);
    QString convertedDdl = convertDdlToDstVersion(ddl);
    if (convertedDdl.trimmed() == ";") // empty query, result of ignored errors in UI
        return true;

    SqlResultsPtr result = dstDb->exec(convertedDdl);
    if (result->isError())
    {
        notifyError(errorMessage.arg(result->getErrorText()));
        qDebug() << "DDL that caused error in DbObjectOrganizer::copySimpleObjectToDb():" << ddl << "\nAfter converting:" << convertedDdl;
        return false;
    }

    return true;
}

QSet<QString> DbObjectOrganizer::resolveReferencedTables(const QString& table, const QList<SqliteCreateTablePtr>& parsedTables)
{
    QSet<QString> tables = SchemaResolver::getFkReferencingTables(table, parsedTables).toSet();
    for (const QString& fkTable : tables)
        tables += SchemaResolver::getFkReferencingTables(fkTable, parsedTables).toSet();

    tables.remove(table); // if it appeared somewhere in the references - we still don't need it here, it's the table we asked by in the first place
    return tables;
}

void DbObjectOrganizer::collectDiffs(const QHash<QString, SchemaResolver::ObjectDetails>& details)
{
    if (srcDb->getVersion() == dstDb->getVersion())
        return;


    int dstVersion = dstDb->getVersion();
    QSet<QString> names = srcTables + srcViews + referencedTables + srcIndexes + srcTriggers;
    for (const QString& name : names)
    {
        if (!details.contains(name))
        {
            qCritical() << "Object named" << name << "not found in details when trying to prepare Diff for copying or moving object.";
            continue;
        }

        versionConverter->reset();
        if (dstVersion == 3)
            versionConverter->convertToVersion3(details[name].ddl);
        else
            versionConverter->convertToVersion2(details[name].ddl);

        diffListToConfirm += versionConverter->getDiffList();
        if (!versionConverter->getErrors().isEmpty())
            errorsToConfirm[name] = versionConverter->getErrors();
    }
}

QString DbObjectOrganizer::convertDdlToDstVersion(const QString& ddl)
{
    if (srcDb->getVersion() == dstDb->getVersion())
        return ddl;

    if (dstDb->getVersion() == 3)
        return versionConverter->convertToVersion3(ddl);
    else
        return versionConverter->convertToVersion2(ddl);
}

void DbObjectOrganizer::collectReferencedTables(const QString& table, const QHash<QString, SqliteQueryPtr>& allParsedObjects)
{
    QList<SqliteCreateTablePtr> parsedTables;
    SqliteCreateTablePtr parsedTable;
    for (SqliteQueryPtr query : allParsedObjects)
    {
        parsedTable = query.dynamicCast<SqliteCreateTable>();
        if (parsedTable)
            parsedTables << parsedTable;
    }

    QSet<QString> tables = resolveReferencedTables(table, parsedTables);
    for (const QString& refTable : tables)
    {
        if (!referencedTables.contains(refTable) && !srcTables.contains(refTable))
            referencedTables << refTable;
    }
}

void DbObjectOrganizer::collectReferencedIndexes(const QString& table)
{
    srcIndexes += srcResolver->getIndexesForTable(table).toSet();
}

void DbObjectOrganizer::collectReferencedTriggersForTable(const QString& table)
{
    srcTriggers += srcResolver->getTriggersForTable(table).toSet();
}

void DbObjectOrganizer::collectReferencedTriggersForView(const QString& view)
{
    srcTriggers += srcResolver->getTriggersForView(view).toSet();
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
    srcResolver->setIgnoreSystemObjects(true);
    dstResolver->setIgnoreSystemObjects(true);
}

void DbObjectOrganizer::emitFinished(bool success)
{
    switch (mode)
    {
        case Mode::COPY_OBJECTS:
        case Mode::PREPARE_TO_COPY_OBJECTS:
            emit finishedDbObjectsCopy(success, srcDb, dstDb);
            break;
        case Mode::PREPARE_TO_MOVE_OBJECTS:
        case Mode::MOVE_OBJECTS:
            emit finishedDbObjectsMove(success, srcDb, dstDb);
            break;
        case Mode::unknown:
            break;
    }
    setExecuting(false);
}

void DbObjectOrganizer::processPreparationFinished()
{
    if (errorsToConfirm.size() > 0 && !conversionErrorsConfimFunction(errorsToConfirm))
    {
        emitFinished(false);
        return;
    }

    if (diffListToConfirm.size() > 0 && !conversionConfimFunction(diffListToConfirm))
    {
        emitFinished(false);
        return;
    }

    if (!resolveNameConflicts())
    {
        emitFinished(false);
        return;
    }

    switch (mode)
    {
        case Mode::PREPARE_TO_COPY_OBJECTS:
            mode = Mode::COPY_OBJECTS;
            break;
        case Mode::PREPARE_TO_MOVE_OBJECTS:
            mode = Mode::MOVE_OBJECTS;
            break;
        case Mode::COPY_OBJECTS:
        case Mode::MOVE_OBJECTS:
        case Mode::unknown:
            qCritical() << "DbObjectOrganizer::processPreparationFinished() called with a not PREPARE mode.";
            emitFinished(false);
            return;
    }

    QThreadPool::globalInstance()->start(this);
}
