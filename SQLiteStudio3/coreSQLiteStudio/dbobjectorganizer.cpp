#include "dbobjectorganizer.h"
#include "db/db.h"
#include "common/utils_sql.h"
#include "services/notifymanager.h"
#include "db/attachguard.h"
#include "common/compatibility.h"
#include <QDebug>
#include <QThreadPool>

DbObjectOrganizer::DbObjectOrganizer()
{
    // Default organizaer denies any referenced objects
    confirmFunction = [](const QStringList&) -> bool {return false;};
    nameConflictResolveFunction = [](QString&) -> bool {return false;};
    conversionConfimFunction = [](const QList<QPair<QString,QString>>&) -> bool {return false;};
    conversionErrorsConfimFunction = [](const QHash<QString,QSet<QString>>&) -> bool {return false;};
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
}

void DbObjectOrganizer::init()
{
    connect(this, SIGNAL(preparetionFinished()), this, SLOT(processPreparationFinished()));
}

void DbObjectOrganizer::copyObjectsToDb(Db* srcDb, const QStringList& objNames, Db* dstDb, bool includeData, bool includeIndexes, bool includeTriggers)
{
    copyOrMoveObjectsToDb(srcDb, toSet(objNames), dstDb, includeData, includeIndexes, includeTriggers, false);
}

void DbObjectOrganizer::moveObjectsToDb(Db* srcDb, const QStringList& objNames, Db* dstDb, bool includeData, bool includeIndexes, bool includeTriggers)
{
    copyOrMoveObjectsToDb(srcDb, toSet(objNames), dstDb, includeData, includeIndexes, includeTriggers, true);
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
    attachName = QString();
    mode = Mode::COPY_OBJECTS;
    srcDb = nullptr;
    dstDb = nullptr;
    srcNames.clear();
    srcTables.clear();
    srcIndexes.clear();
    srcTriggers.clear();
    srcViews.clear();
    renamed.clear();
    srcTable = QString();
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
    StrHash<SchemaResolver::ObjectDetails> details = srcResolver->getAllObjectDetails();
    for (const QString& srcName : srcNames)
    {
        if (!details.contains(srcName))
        {
            qDebug() << "Object" << srcName << "not found in source database, skipping.";
            continue;
        }

        switch (details[srcName].type)
        {
            case SchemaResolver::TABLE:
                srcTables << srcName;
                collectReferencedTables(srcName);
                collectReferencedIndexes(srcName);
                collectReferencedTriggersForTable(srcName);
                break;
            case SchemaResolver::INDEX:
                break;
            case SchemaResolver::TRIGGER:
                break;
            case SchemaResolver::VIEW:
                srcViews << srcName;
                collectReferencedTriggersForView(srcName);
                break;
            case SchemaResolver::ANY:
                qCritical() << "Unhandled type in DbObjectOrganizer::processPreparation():" << SchemaResolver::objectTypeToString(details[srcName].type);
                break;
        }
    }

    if (referencedTables.size() > 0 && !execConfirmFunctionInMainThread(referencedTables.values()))
        referencedTables.clear();

    for (const QString& srcTable : referencedTables)
    {
        collectReferencedIndexes(srcTable);
        collectReferencedTriggersForTable(srcTable);
    }

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

    if (!setFkEnabled(false))
        return false;

    bool res = processAllWithFkDisabled();
    setFkEnabled(true);
    return res;
}

bool DbObjectOrganizer::processAllWithFkDisabled()
{
    // Attaching target db if needed
    AttachGuard attach;
    if (useAttachingApproach())
    {
        attach = srcDb->guardedAttach(dstDb, true);
        attachName = attach->getName();
    }

    if (!srcDb->begin())
    {
        // TODO message
        return false;
    }

    if (!dstDb->begin())
    {
        // TODO message
        srcDb->rollback();
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

    if (!dstDb->commit())
    {
        // notifyError(tr("Could not commit transaction in database '%1'.").arg(dstDb->getName())); // TODO this is in another thread, cannot use notifyError
        dstDb->rollback();
        srcDb->rollback();
        return false;
    }

    if (!res)
    {
        srcDb->rollback();
        dstDb->rollback();
        return false;
    }

    if (!srcDb->commit())
    {
        // TODO message - this can happen also for attached db operations, so also for creating objects in dstDb, so this affects not only srcDb, but also dstDb
        srcDb->rollback();
        return false;
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
    if (renamed.contains(table) || useAttachingApproach())
    {
        SqliteQueryPtr parsedObject = srcResolver->getParsedObject(table, SchemaResolver::TABLE);
        SqliteCreateTablePtr createTable = parsedObject.dynamicCast<SqliteCreateTable>();
        if (!createTable)
        {
            qCritical() << "Could not parse table while copying:" << table << ", ddl:" << srcResolver->getObjectDdl(table, SchemaResolver::TABLE);
            notifyError(tr("Error while creating table in target database: %1").arg(tr("Could not parse table.")));
            return false;
        }

        if (renamed.contains(table))
            targetTable = renamed[table];

        createTable->table = targetTable;
        if (useAttachingApproach())
            createTable->database = attachName;

        createTable->rebuildTokens();
        ddl = createTable->detokenize();
    }
    else
    {
        ddl = srcResolver->getObjectDdl(table, SchemaResolver::TABLE);
    }

    if (ddl.trimmed() == ";") // empty query, result of ignored errors in UI
        return true;

    SqlQueryPtr result;

    if (!useAttachingApproach())
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
    if (!useAttachingApproach())
    {
        notifyInfo(tr("Database %1 could not be attached to database %2, so the data of table %3 will be copied "
                      "with SQLiteStudio as a mediator. This method can be slow for huge tables, so please be patient.")
                   .arg(dstDb->getName(), srcDb->getName(), srcTable));

        res = copyDataAsMiddleware(targetTable);
    }
    else
    {
        res = copyDataUsingAttach(targetTable);
    }
    return res;
}

bool DbObjectOrganizer::copyDataAsMiddleware(const QString& table)
{
    static_qstring(selectTpl, "SELECT %1 FROM %2");
    static_qstring(insertTpl, "INSERT INTO %1 (%2) VALUES (%3)");

    QStringList srcColumns = srcResolver->getTableColumns(srcTable, true);
    QString srcColumnsStr = srcColumns.join(", ");
    QString wrappedSrcTable = wrapObjIfNeeded(srcTable);
    SqlQueryPtr results = srcDb->prepare(selectTpl.arg(srcColumnsStr, wrappedSrcTable));
    if (!results->execute())
    {
        notifyError(tr("Error while copying data for table %1: %2").arg(table, results->getErrorText()));
        return false;
    }

    QStringList argPlaceholderList;
    for (int i = 0, total = srcColumns.size(); i < total; ++i)
        argPlaceholderList << "?";

    QString wrappedDstTable = wrapObjIfNeeded(table);
    QString sql = insertTpl.arg(wrappedDstTable, srcColumnsStr, argPlaceholderList.join(", "));
    SqlQueryPtr insertQuery = dstDb->prepare(sql);

    SqlResultsRowPtr row;
    int i = 0;
    while (results->hasNext())
    {
        row = results->next();
        if (!row)
        {
            notifyError(tr("Error while copying data to table %1: %2").arg(table, results->getErrorText()));
            return false;
        }

        insertQuery->setArgs(row->valueList());
        if (!insertQuery->execute())
        {
            notifyError(tr("Error while copying data to table %1: %2").arg(table, insertQuery->getErrorText()));
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

bool DbObjectOrganizer::copyDataUsingAttach(const QString& table)
{
    static_qstring(insertTpl, "INSERT INTO %1.%2 (%3) SELECT %3 FROM %4");

    QString wrappedSrcTable = wrapObjIfNeeded(srcTable);
    QString wrappedDstTable = wrapObjIfNeeded(table);
    QStringList srcColumns = srcResolver->getTableColumns(srcTable, true);
    QString srcColumnsStr = wrapObjNamesIfNeeded(srcColumns).join(", ");
    SqlQueryPtr results = srcDb->exec(insertTpl.arg(attachName, wrappedDstTable, srcColumnsStr, wrappedSrcTable));
    if (results->isError())
    {
        notifyError(tr("Error while copying data to table %1: %2").arg(table, results->getErrorText()));
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
    QString wrappedSrcObj = wrapObjIfNeeded(name);
    SqlQueryPtr results = srcDb->exec("DROP " + type + " " + wrappedSrcObj);
    if (results->isError())
    {
        notifyWarn(tr("Error while dropping source view %1: %2\nTables, indexes, triggers and views copied to database %3 will remain.")
                   .arg(name, results->getErrorText(), dstDb->getName()));
    }
}

bool DbObjectOrganizer::copyViewToDb(const QString& view)
{
    return copySimpleObjectToDb(view, tr("Error while creating view in target database: %1"), SchemaResolver::VIEW);
}

bool DbObjectOrganizer::copyIndexToDb(const QString& index)
{
    return copySimpleObjectToDb(index, tr("Error while creating index in target database: %1"),SchemaResolver::INDEX);
}

bool DbObjectOrganizer::copyTriggerToDb(const QString& trigger)
{
    return copySimpleObjectToDb(trigger, tr("Error while creating trigger in target database: %1"), SchemaResolver::TRIGGER);
}

bool DbObjectOrganizer::copySimpleObjectToDb(const QString& name, const QString& errorMessage, SchemaResolver::ObjectType objectType)
{
    QString ddl = srcResolver->getObjectDdl(name, objectType);
    if (ddl.trimmed() == ";") // empty query, result of ignored errors in UI
        return true;

    ddl = processSimpleObjectAttachNameAndRename(name, ddl);
    if (ddl.isNull())
        return false;

    SqlQueryPtr result;
    if (!useAttachingApproach())
        result = dstDb->exec(ddl);
    else
        result = srcDb->exec(ddl); // uses attachName to create object in attached db

    if (result->isError())
    {
        notifyError(errorMessage.arg(result->getErrorText()));
        qDebug() << "DDL that caused error in DbObjectOrganizer::copySimpleObjectToDb():" << ddl;
        return false;
    }

    return true;
}

void DbObjectOrganizer::collectReferencedTables(const QString& table)
{
    QStringList tables = srcResolver->getFkReferencedTables(table);
    for (const QString& refTable : tables)
    {
        if (!referencedTables.contains(refTable) && !srcTables.contains(refTable))
            referencedTables << refTable;
    }
}

void DbObjectOrganizer::collectReferencedIndexes(const QString& table)
{
    srcIndexes += toSet(srcResolver->getIndexesForTable(table));
}

void DbObjectOrganizer::collectReferencedTriggersForTable(const QString& table)
{
    srcTriggers += toSet(srcResolver->getTriggersForTable(table));
}

void DbObjectOrganizer::collectReferencedTriggersForView(const QString& view)
{
    srcTriggers += toSet(srcResolver->getTriggersForView(view));
}

bool DbObjectOrganizer::setFkEnabled(bool enabled)
{
    Db* theDb = useAttachingApproach() ? srcDb : dstDb;
    SqlQueryPtr result = theDb->exec(QString("PRAGMA foreign_keys = %1").arg(enabled ? "on" : "off"));
    if (result->isError())
    {
        // notifyError(tr("Error while executing PRAGMA on target database: %1").arg(result->getErrorText())); // TODO this is in another thread, cannot use notifyError
        return false;
    }
    return true;
}

bool DbObjectOrganizer::isInterrupted()
{
    QMutexLocker locker(&interruptMutex);
    return interrupted;
}

bool DbObjectOrganizer::useAttachingApproach() const
{
    return !attachName.isNull() || (
                srcDb->getTypeClassName() == dstDb->getTypeClassName() &&
                !(referencedTables + srcTables).isEmpty()
            );
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

bool DbObjectOrganizer::execConfirmFunctionInMainThread(const QStringList& tables)
{
    bool res;
    bool invokation = QMetaObject::invokeMethod(this, "confirmFunctionSlot", Qt::BlockingQueuedConnection,
                                                Q_RETURN_ARG(bool, res), Q_ARG(QStringList, tables));

    if (!invokation)
    {
        qCritical() << "Could not call DbObjectOrganizer::confirmFunctionSlot() between threads!";
        return false;
    }

    return res;
}

QString DbObjectOrganizer::processSimpleObjectAttachNameAndRename(const QString& objName, const QString& ddl)
{
    if (!useAttachingApproach() && !renamed.contains(objName))
        return ddl;

    Parser parser;
    if (!parser.parse(ddl))
    {
        qDebug() << "Parsing error while copying or moving object:" << objName << ", details:" << parser.getErrorString();
        notifyError(tr("Could not parse object '%1' in order to move or copy it.").arg(objName));
        return QString();
    }

    if (parser.getQueries().isEmpty())
    {
        qDebug() << "Empty queries from parser while copying or moving object:" << objName;
        notifyError(tr("Could not parse object '%1' in order to move or copy it.").arg(objName));
        return QString();
    }

    SqliteQueryPtr query = parser.getQueries().first();
    SqliteDdlWithDbContextPtr ddlWithDb = query.dynamicCast<SqliteDdlWithDbContext>();
    if (!ddlWithDb)
    {
        qDebug() << "Not instance of SqliteDdlWithDbContext while copying or moving object:" << objName << ", it's type is:" << (int)query->queryType;
        notifyError(tr("Could not parse object '%1' in order to move or copy it.").arg(objName));
        return QString();
    }

    if (useAttachingApproach())
        ddlWithDb->setTargetDatabase(attachName);

    if (renamed.contains(objName))
        ddlWithDb->setObjectName(renamed[objName]);

    query->rebuildTokens();
    return query->tokens.detokenize();
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

bool DbObjectOrganizer::confirmFunctionSlot(const QStringList& tables)
{
    return confirmFunction(tables);
}
