#include "erdchange.h"
#include "erdeffectivechangemerger.h"
#include "services/dbmanager.h"
#include "uidebug.h"
#include "services/notifymanager.h"
#include "erdchangedeleteentity.h"
#include "erdchangemodifyentity.h"
#include "erdchangenewentity.h"
#include "tablemodifier.h"
#include "diff/diff_match_patch.h"
#include "db/chainexecutor.h"
#include "changes/erdchangecomposite.h"
#include "common/unused.h"

ErdEffectiveChangeMerger::ErdEffectiveChangeMerger(const QStringList& schemaBase, const QString& dbName) :
    schemaBase(schemaBase), dbName(dbName)
{
}

QList<ErdEffectiveChange> ErdEffectiveChangeMerger::merge(const QList<ErdChange*>& changes)
{
    static_qstring(savepointTpl, "SAVEPOINT '%1'");
    static_qstring(rollbackToTpl, "ROLLBACK TO '%1'");
    static_qstring(disableFkTpl, "PRAGMA foreign_keys = 0;");
    static_qstring(startingSavepoint, "erd_change_compacting_start");

    ddlCacheByChangeId.clear();

    QScopedPointer<Db> referenceDb(createMemDbWithSchema(schemaBase, dbName));
    QScopedPointer<Db> workingDb(createMemDbWithSchema(schemaBase, dbName));
    if (!referenceDb || !workingDb)
    {
        notifyError(QObject::tr("Failed to create in-memory databases required for change compacting."));
        return {};
    }

    // Flatten and transform to effective changes
    QList<ErdChange*> flat = flatten(changes);
    int initialChangeCount = flat.size();
    qDebug() << "Starting compacting of" << initialChangeCount << "changes.";

    QList<ErdEffectiveChange> result = flat
            | FILTER(chg, {return chg->isDdlChange();})
            | MAP(chg, {return chg->toEffectiveChange();});

    if (result.isEmpty())
        return {};

    // Prepare initial state in both DBs.
    // FK enforcing is disabled prior to any transactions - otherwise it would have no effect.
    referenceDb->exec(disableFkTpl);
    workingDb->exec(disableFkTpl);
    referenceDb->exec(savepointTpl.arg(startingSavepoint));
    workingDb->exec(savepointTpl.arg(startingSavepoint));

    bool modified = true;
    // While something was modified in last iteration, there is still a chance to compact more
    while (modified)
    {
        modified = false;

        // Restart schemas for next iteration
        referenceDb->exec(rollbackToTpl.arg(startingSavepoint));
        workingDb->exec(rollbackToTpl.arg(startingSavepoint));

        QList<ErdEffectiveChange> currentIterationResults;
        for (int idx = 0, total = result.size(); idx < total; ++idx)
        {
            int originalIdx = idx;
            ErdEffectiveChange newChange = merge(result, idx, referenceDb.data(), workingDb.data());
            if (newChange.isValid())
            {
                executeOneChangeOnBothDbs(newChange, referenceDb.data(), workingDb.data());
                currentIterationResults << newChange;
                modified |= (originalIdx != idx);
            }
            else if (newChange.getType() == ErdEffectiveChange::NOOP)
                modified |= true;
        }
        result = currentIterationResults;
    }

    int reducedChangeCount = result.size();
    qDebug() << "Compacting completed. Reduced change count from"
             << initialChangeCount << "to" << reducedChangeCount << ".";

    return result;
}

Db* ErdEffectiveChangeMerger::createMemDbWithSchema(const QStringList& schemaDdls, const QString& name)
{
    Db* newDb = DBLIST->createInMemDb();
    newDb->setName(name); // same name as original, so that all functions/collations/extensions are loaded when open
    if (!newDb->openQuiet())
    {
        qCritical() << "Failed to open in-memory database required for ERD editor! Db error:" << newDb->getErrorText();
        return nullptr;
    }

    // Now copy schema to memdb
    for (const QString& ddl : schemaDdls)
        newDb->exec(ddl);

    return newDb;
}

QStringList ErdEffectiveChangeMerger::readDbSchema(Db* db)
{
    SqlQueryPtr tableResults = db->exec("SELECT sql FROM sqlite_schema WHERE type = 'table' AND name NOT LIKE 'sqlite_%'");
    SqlQueryPtr viewResults = db->exec("SELECT sql FROM sqlite_schema WHERE type = 'view' AND name NOT LIKE 'sqlite_%'");
    SqlQueryPtr triggerResults = db->exec("SELECT sql FROM sqlite_schema WHERE type = 'trigger' AND name NOT LIKE 'sqlite_%'");
    SqlQueryPtr indexResults = db->exec("SELECT sql FROM sqlite_schema WHERE type = 'index' AND name NOT LIKE 'sqlite_%'");

    QStringList output;
    for (const SqlQueryPtr& results : {tableResults, viewResults, triggerResults, indexResults})
    {
        for (SqlResultsRowPtr& row : results->getAll())
            output << row->value("sql").toString();
    }
    return output;
}

ErdEffectiveChange ErdEffectiveChangeMerger::merge(const QList<ErdEffectiveChange>& workingList, int& idx,
                                            Db* referenceDb, Db* workingDb)
{
    switch (workingList[idx].getType())
    {
        case ErdEffectiveChange::CREATE:
            return mergeToCreateChange(workingList, idx, referenceDb, workingDb);
        case ErdEffectiveChange::DROP:
            return mergeToDropChange(workingList, idx, referenceDb, workingDb);
        case ErdEffectiveChange::MODIFY:
            return mergeToModifyChange(workingList, idx, referenceDb, workingDb);
        case ErdEffectiveChange::INVALID:
            break;
        case ErdEffectiveChange::NOOP:
            return workingList[idx];
        case ErdEffectiveChange::RAW:
            return workingList[idx];
    }
    qCritical() << "ErdEffectiveChangeMerger::merge: Unsupported effective change type" << int(workingList[idx].getType())
                << ", desc:" << workingList[idx].getDescription();
    return workingList[idx];
}

ErdEffectiveChange ErdEffectiveChangeMerger::mergeToCreateChange(const QList<ErdEffectiveChange>& theList,
                                                          int& idx, Db* referenceDb, Db* workingDb)
{
    if (idx + 1 >= theList.size())
        return theList[idx];

    // CREATE, MODIFY, MODIFY, ... -> CREATE
    if (theList[idx + 1].getType() == ErdEffectiveChange::MODIFY)
        return mergeMultipleModifyToCreateChange(theList, idx, referenceDb, workingDb);

    // CREATE, DROP -> NOOP
    if (theList[idx + 1].getType() == ErdEffectiveChange::DROP)
        return mergeDropToCreateChange(theList, idx, referenceDb, workingDb);

    return theList[idx];
}

ErdEffectiveChange ErdEffectiveChangeMerger::mergeMultipleModifyToCreateChange(const QList<ErdEffectiveChange>& theList,
                                                          int& idx, Db* referenceDb, Db* workingDb)
{
    // General strategy here is to first perform mergers ahead if possible.
    // Then take result and try to merge with this CREATE.
    int targetIdx = idx + 1;
    ErdEffectiveChange changeToMerge;

    // Calculate merged MODIFY ahead
    {
        // Temporarily apply the starting CREATE, so that subsequent MODIFY's can be merged properly
        auto createRollback = scopedTxRollback(referenceDb, workingDb);
        executeOneChangeOnBothDbs(theList[idx], referenceDb, workingDb);
        changeToMerge = mergeToModifyChange(theList, targetIdx, referenceDb, workingDb);
    }

    ErdEffectiveChange createChange = theList[idx];
    if (changeToMerge.getType() == ErdEffectiveChange::MODIFY &&
        changeToMerge.getTableName() == createChange.getAfter()->table)
    {
        // Merge CREATE + MODIFY into single CREATE
        SqliteCreateTablePtr after = changeToMerge.getAfter();
        QString description = ErdChangeNewEntity::defaultDescription(after->table);
        ErdEffectiveChange mergedChange = ErdEffectiveChange::create(after, description);
        bool success = testAgainstOriginal(mergedChange, theList.mid(idx, targetIdx - idx + 1), referenceDb, workingDb);
        if (success)
        {
            idx = targetIdx; // Move idx ahead to point to last merged change
            qDebug() << "Merged CREATE and MODIFY changes for table" << after->table << "into one CREATE.";
            return mergedChange;
        }
    }

    if (changeToMerge.getType() == ErdEffectiveChange::DROP &&
        changeToMerge.getTableName() == createChange.getAfter()->table)
    {
        // Subsequent changes were merged into DROP, so we have case of: CREATE + (MODIFY... + DROP -> DROP)
        // Merge CREATE + (MODIFY... + DROP -> DROP) into single NOOP
        ErdEffectiveChange mergedChange = ErdEffectiveChange::noop();
        bool success = testAgainstOriginal(mergedChange, theList.mid(idx, targetIdx - idx + 1), referenceDb, workingDb);
        if (success)
        {
            idx = targetIdx; // Move idx ahead to point to last merged change
            qDebug() << "Merged CREATE and DROP changes for table" << changeToMerge.getTableName() << "into NOOP.";
            return mergedChange;
        }
    }
    return theList[idx];
}

ErdEffectiveChange ErdEffectiveChangeMerger::mergeDropToCreateChange(const QList<ErdEffectiveChange>& theList,
                                                          int& idx, Db* referenceDb, Db* workingDb)
{
    // Temporarily apply the starting CREATE, so that subsequent DROP can be merged properly
    auto createRollback = scopedTxRollback(referenceDb, workingDb);
    executeOneChangeOnBothDbs(theList[idx], referenceDb, workingDb);

    // If we DROP right after CREATE, we can merge both into NOOP.
    // First perform any DROP mergers ahead if possible.
    int targetIdx = idx + 1;
    ErdEffectiveChange changeToMerge = mergeToDropChange(theList, targetIdx, referenceDb, workingDb);
    if (changeToMerge.getType() == ErdEffectiveChange::DROP &&
        changeToMerge.getTableName() == theList[idx].getAfter()->table)
    {
        // Merged CREATE + DROP into NOOP
        ErdEffectiveChange mergedChange = ErdEffectiveChange::noop();
        bool success = testAgainstOriginal(mergedChange, {}, referenceDb, workingDb);
        if (success)
        {
            qDebug() << "Merged CREATE and DROP changes for table" << theList[idx].getAfter()->table << "into NOOP.";
            idx = targetIdx; // Move idx ahead to point to last merged change
            return mergedChange;
        }
    }
    return theList[idx];
}

ErdEffectiveChange ErdEffectiveChangeMerger::mergeToDropChange(const QList<ErdEffectiveChange>& theList,
                                                        int& idx, Db* referenceDb, Db* workingDb)
{
    int localIdx = idx;
    QStringList changesToProcess;
    QHash<QString, QSet<QString>> idToAffectedTables;
    QSet<QString> affectedTablesSoFar;
    QStringList tablesToDropSoFar;

    {
        auto rollbackTx = scopedTxRollback(referenceDb, workingDb);
        while (localIdx < theList.size())
        {
            ErdEffectiveChange next = theList[localIdx];
            if (next.getType() != ErdEffectiveChange::DROP)
                break;

            // To populate aftermath cache if not done yet we need to execute each change,
            // so that all referenced tables are discovered properly.
            executeOnDb(next, referenceDb);

            QString id = next.getId();
            changesToProcess << id;

            tablesToDropSoFar << next.getTableName();
            affectedTablesSoFar.unite(toSet(aftermathByChangeId.value(id).modifiedTables));
            idToAffectedTables.insert(id, affectedTablesSoFar);

            localIdx++;
        }

        if (localIdx >= theList.size())
        {
            // All changes were DROPs, but because of that the localIdx was incremented beyond the list size.
            localIdx--;
        }
    }

    while (!changesToProcess.isEmpty())
    {
        QString id = changesToProcess.last();
        if (contains(tablesToDropSoFar, idToAffectedTables[id], Qt::CaseInsensitive))
        {
            // All affected tables are among the ones being dropped - we can merge this DROP
            break;
        }
        tablesToDropSoFar.removeLast();
        changesToProcess.removeLast();
        localIdx--;
    }

    int stepsToMerge = localIdx - idx;
    if (stepsToMerge > 0)
    {
        static_qstring(rawDropDdl, "DROP TABLE %1;");
        QStringList ddls;
        for (int i = idx; i <= localIdx; i++)
            ddls << rawDropDdl.arg(wrapObjIfNeeded(theList[i].getTableName()));

        QString desc = QObject::tr("Drop tables: %1", "ERD editor").arg(tablesToDropSoFar.join(", "));
        ErdEffectiveChange mergedChange = ErdEffectiveChange::raw(ddls, desc);
        bool success = testAgainstOriginal(mergedChange, theList.mid(idx, stepsToMerge + 1), referenceDb, workingDb);
        if (success)
        {
            qDebug() << "Merged multiple DROP changes for tables" << tablesToDropSoFar << "into RAW.";
            idx += stepsToMerge;
            return mergedChange;
        }
    }

    return theList[idx];
}

ErdEffectiveChange ErdEffectiveChangeMerger::mergeToModifyChange(const QList<ErdEffectiveChange>& theList,
                                                          int& idx, Db* referenceDb, Db* workingDb)
{
    const int initialIdx = idx;

    // MODIFY(a->b), MODIFY(b->a) -> NOOP
    auto ddlBefore = theList[idx].getBefore()->detokenize();
    auto ddlAfter = theList[idx].getAfter()->detokenize();
    if (ddlBefore == ddlAfter)
    {
        qDebug() << "Removed MODIFY change for table" << theList[idx].getTableName() << "as it produces no effective change.";
        return ErdEffectiveChange::noop();
    }

    // MODIFY, MODIFY, MODIFY, ... -> MODIFY
    ErdEffectiveChange resultChange = mergeMultipleModifyToOne(theList, idx, referenceDb, workingDb);
    if (idx > initialIdx)
        return resultChange;

    // MODIFY, MODIFY, DROP -> DROP
    resultChange = mergeModifyToDropAhead(theList, idx, referenceDb, workingDb);
    if (idx > initialIdx)
        return resultChange;

    return resultChange;
}

ErdEffectiveChange ErdEffectiveChangeMerger::mergeMultipleModifyToOne(const QList<ErdEffectiveChange>& theList, int& idx, Db* referenceDb, Db* workingDb)
{
    // First, try to look how many MODIFY's in a row we have for the same table
    int stepsToMerge = 0;
    QString currentTableName = theList[idx].getTableName();
    while ((idx + stepsToMerge + 1) < theList.size())
    {
        ErdEffectiveChange next = theList[idx + stepsToMerge + 1];
        if (next.getType() != ErdEffectiveChange::MODIFY || next.getTableName() != currentTableName)
            break;

        currentTableName = next.getAfter()->table;
        stepsToMerge++;
    }

    // Now, try to merge as many as possible starting from the maximum found
    if (stepsToMerge > 0)
    {
        SqliteCreateTablePtr before = theList[idx].getBefore();
        QString description = ErdChangeModifyEntity::defaultDescription(before->table);
        while (stepsToMerge > 0)
        {
            SqliteCreateTablePtr after = theList[idx + stepsToMerge].getAfter();
            ErdEffectiveChange mergedChange = ErdEffectiveChange::modify(before, after, description);
            bool success = testAgainstOriginal(mergedChange, theList.mid(idx, stepsToMerge + 1), referenceDb, workingDb);
            if (success)
            {
                idx += stepsToMerge;
                qDebug() << "Merged" << (stepsToMerge + 1) << "MODIFY changes for table" << before->table << "into one MODIFY.";
                return mergedChange;
            }
            stepsToMerge--;
        }
        // Nothing could be merged.
    }
    return theList[idx];
}

ErdEffectiveChange ErdEffectiveChangeMerger::mergeModifyToDropAhead(const QList<ErdEffectiveChange>& theList, int& idx, Db* referenceDb, Db* workingDb)
{
    if (idx + 1 >= theList.size())
        return theList[idx];

    QString beforeTableName = theList[idx].getTableName();
    QString afterTableName = theList[idx].getAfter()->table;
    ErdEffectiveChange next = theList[idx + 1];
    if (next.getType() == ErdEffectiveChange::DROP && next.getTableName() == afterTableName)
    {
        QString changeDesc = ErdChangeDeleteEntity::defaultDescription(beforeTableName);
        ErdEffectiveChange mergedChange = ErdEffectiveChange::drop(beforeTableName, changeDesc);
        bool success = testAgainstOriginal(mergedChange, theList.mid(idx, 2), referenceDb, workingDb);
        if (success)
        {
            idx++;
            qDebug() << "Merged MODIFY and DROP changes for table" << beforeTableName << "into one DROP.";
            return mergedChange;
        }
    }

    return theList[idx];
}

QString ErdEffectiveChangeMerger::schemaSnapshot(Db* db)
{
    static_qstring(sql, "SELECT string_agg(sql, ';' || char(10)) FROM sqlite_master WHERE sql IS NOT NULL ORDER BY type, name");
    SqlQueryPtr query = db->exec(sql);
    return query->getSingleCell().toString();
}

bool ErdEffectiveChangeMerger::testAgainstOriginal(ErdEffectiveChange mergedChange, const QList<ErdEffectiveChange>& referenceChanges,
                                               Db* referenceDb, Db* workingDb)
{
    ChainExecutor executor;
    executor.setTransaction(false);
    executor.setAsync(false);
    executor.setDisableForeignKeys(true);
    executor.setDisableObjectDropsDetection(true);

    // Always rollback whatever is done in this method
    auto rollbackGuard = scopedTxRollback(referenceDb, workingDb);

    // Apply related changes to reference DB
    executor.setDb(referenceDb);
    for (const ErdEffectiveChange& chg : referenceChanges)
    {
        // List of changes must be executed one by one, so that TableModifier
        // can work on up-to-date schema per each change.
        QStringList referenceDdls = getDdlForChange(chg, referenceDb);
        executor.setQueries(referenceDdls);
        executor.exec();
        if (!executor.getSuccessfulExecution())
        {
            qWarning() << QString("Failed to apply related change \"%1\" to reference DB during merging test:").arg(chg.getDescription())
                       << executor.getErrorsMessages();
            return false;
        }
    }

    // Apply merged change to working DB
    QStringList mergedDdl = getDdlForChange(mergedChange, workingDb);
    if (!mergedDdl.isEmpty()) // can be empty in case of NOOP
    {
        executor.setDb(workingDb);
        executor.setQueries(mergedDdl);
        executor.exec();
        if (!executor.getSuccessfulExecution())
        {
            qWarning() << "Failed to apply merged change to working DB during merging test:"
                        << executor.getErrorsMessages();
            return false;
        }
    }

    QString referenceSchema = schemaSnapshot(referenceDb);
    QString workingSchema = schemaSnapshot(workingDb);

    if (referenceSchema == workingSchema)
        return true;

    qWarning() << "ErdEffectiveChangeMerger::testAgainstOriginal: Merged change schema does not match reference schema.";
    if (isDebugEnabled())
        debugSnapshotDiff(referenceSchema, workingSchema);

    return false;
}

void ErdEffectiveChangeMerger::executeOneChangeOnBothDbs(ErdEffectiveChange change, Db* referenceDb, Db* workingDb)
{
    static_qstring(savepointTpl, "SAVEPOINT '%1'");
    static_qstring(rollbackToTpl, "ROLLBACK TO '%1'");
    static_qstring(releaseSavepointTpl, "RELEASE '%1'");

    QString savepoint = QUuid::createUuid().toString(QUuid::WithoutBraces);
    referenceDb->exec(savepointTpl.arg(savepoint));
    workingDb->exec(savepointTpl.arg(savepoint));

    ChainExecutor executor;
    executor.setTransaction(false);
    executor.setAsync(false);
    executor.setDisableForeignKeys(true);
    executor.setDisableObjectDropsDetection(true);

    QStringList ddl = getDdlForChange(change, workingDb);
    executor.setQueries(ddl);
    for (Db* db : {referenceDb, workingDb})
    {
        executor.setDb(db);
        executor.exec();
        if (!executor.getSuccessfulExecution())
        {
            qCritical() << "Failed to apply change to DB during merging test:"
                        << executor.getErrorsMessages();

            workingDb->exec(rollbackToTpl.arg(savepoint));
            referenceDb->exec(rollbackToTpl.arg(savepoint));
            return;
        }
    }
    workingDb->exec(releaseSavepointTpl.arg(savepoint));
    referenceDb->exec(releaseSavepointTpl.arg(savepoint));
}

void ErdEffectiveChangeMerger::executeOnDb(ErdEffectiveChange change, Db* db)
{
    ChainExecutor executor;
    executor.setTransaction(false);
    executor.setAsync(false);
    executor.setDisableForeignKeys(true);
    executor.setDisableObjectDropsDetection(true);

    QStringList ddl = getDdlForChange(change, db);
    executor.setQueries(ddl);
    executor.setDb(db);
    executor.exec();
    if (!executor.getSuccessfulExecution())
        qCritical() << "Failed to execute change on DB:" << executor.getErrorsMessages();
}

QStringList ErdEffectiveChangeMerger::getDdlForChange(const ErdEffectiveChange& change, Db* db)
{
    if (ddlCacheByChangeId.contains(change.getId()))
        return ddlCacheByChangeId[change.getId()];

    TableModifierAftermath aftermath;
    QStringList ddl = generateDdl(change, db, aftermath);
    ddlCacheByChangeId.insert(change.getId(), ddl);
    aftermathByChangeId.insert(change.getId(), aftermath);
    return ddl;
}

QStringList ErdEffectiveChangeMerger::generateDdl(const ErdEffectiveChange& change, Db* db, TableModifierAftermath& aftermath)
{
    switch (change.getType())
    {
        case ErdEffectiveChange::CREATE:
        {
            return {change.getAfter()->detokenize()};
        }
        case ErdEffectiveChange::DROP:
        {
            TableModifier modifier(db, change.getTableName());
            modifier.setDisableFkEnforcement(false);
            modifier.dropTable();
            aftermath.modifiedTables = modifier.getModifiedTables();
            aftermath.modifiedViews = modifier.getModifiedViews();
            return modifier.getGeneratedSqls();
        }
        case ErdEffectiveChange::MODIFY:
        {
            TableModifier modifier(db, change.getBefore()->table, change.getBefore());
            modifier.setDisableFkEnforcement(false);
            modifier.alterTable(change.getAfter());
            aftermath.modifiedTables = modifier.getModifiedTables();
            aftermath.modifiedViews = modifier.getModifiedViews();
            return modifier.getGeneratedSqls();
        }
        case ErdEffectiveChange::RAW:
            return change.getRawDdl();
        case ErdEffectiveChange::INVALID:
            break;
        case ErdEffectiveChange::NOOP:
            return {};
    }
    qCritical() << "ErdEffectiveChange::buildDdl: Unknown type" << static_cast<int>(change.getType())
                << ", desc:" << change.getDescription();

    return {};
}

QStringList ErdEffectiveChangeMerger::getDdlForChange(const ErdEffectiveChange& change) const
{
    if (ddlCacheByChangeId.contains(change.getId()))
        return ddlCacheByChangeId[change.getId()];

    return {};
}

void ErdEffectiveChangeMerger::debugSnapshotDiff(const QString& refSchema, const QString& workingSchema)
{
    diff_match_patch diff;
    QList<Diff> diffs = diff.diff_main(refSchema, workingSchema);
    for (const Diff& d : diffs)
    {
        switch (d.operation)
        {
            case DELETE:
                qDebug() << "Compacting deleted:" << d.text;
                break;
            case EQUAL:
                qDebug() << "Compacting equal:  " << d.text;
                break;
            case INSERT:
                qDebug() << "Compacting added:  " << d.text;
                break;
        }
    }
}

QList<ErdChange*> ErdEffectiveChangeMerger::flatten(const QList<ErdChange*>& changes)
{
    QList<ErdChange*> result;
    for (auto&& chg : changes)
    {
        ErdChangeComposite* composite = dynamic_cast<ErdChangeComposite*>(chg);
        if (composite)
            result += flatten(composite->getChanges());
        else
            result << chg;
    }
    return result;
}
