#include "tablemodifier.h"
#include "common/utils_sql.h"
#include "parser/parser.h"
#include "schemaresolver.h"
#include "parser/ast/sqlitecreateindex.h"
#include "parser/ast/sqlitecreatetrigger.h"
#include "parser/ast/sqlitecreateview.h"
#include "parser/ast/sqliteselect.h"
#include "parser/ast/sqliteupdate.h"
#include "parser/ast/sqliteinsert.h"
#include "parser/ast/sqlitedelete.h"
#include "common/unused.h"
#include <QDebug>

// TODO no attach/temp db name support in this entire class
// mainly in calls to schema resolver, but maybe other stuff too

TableModifier::TableModifier(Db* db, const QString& table) :
    db(db),
    table(table)
{
    init();
}

TableModifier::TableModifier(Db* db, const QString& database, const QString& table) :
    db(db),
    database(database),
    table(table)
{
    init();
}

void TableModifier::alterTable(SqliteCreateTablePtr newCreateTable)
{
    tableColMap = newCreateTable->getModifiedColumnsMap(true);
    existingColumns = newCreateTable->getColumnNames();
    newName = newCreateTable->table;

    sqls << "PRAGMA foreign_keys = 0;";

    handleFkConstrains(newCreateTable.data(), createTable->table, newName);

    QString tempTableName;
    bool doCopyData = !getColumnsToCopyData(newCreateTable).isEmpty();
    if (table.compare(newName, Qt::CaseInsensitive) == 0)
        tempTableName = renameToTemp(doCopyData);

    newCreateTable->rebuildTokens();
    sqls << newCreateTable->detokenize();
    if (doCopyData)
        copyDataTo(newCreateTable);

    handleFks();

    // If temp table was created, it means that table name hasn't changed. In that case we need to cleanup temp table (drop it).
    // Otherwise, the table name has changed, therefor there still remains the old table which we copied data from - we need to drop it here.
    sqls << QString("DROP TABLE %1;").arg(wrapObjIfNeeded(tempTableName.isNull() ? originalTable : tempTableName));

    handleIndexes();
    handleTriggers();
    handleViews();

    sqls << "PRAGMA foreign_keys = 1;";
}

void TableModifier::renameTo(const QString& newName, bool doCopyData)
{
    if (!createTable)
        return;

    // Firstly, using ALTER TABLE RENAME TO is not a good solution here, because it automatically renames all occurrences in REFERENCES,
    // which we don't want, because we rename a lot to temporary tables and drop them.
    //
    // Secondly, we need to identify if a table has column with "reserved literal" used as column name - i.e. "true" or "false".
    // This is allowed by SQLite, but it's treated strangely by SQLite. In many cases result column of such name is renamed to columnN.
    // Example of such case when reserved literal column is used in source table of CREATE TABLE trg AS SELECT * FROM src;
    // It was identified during investigation of #5065.

    bool hasReservedColName = false;
    for (SqliteCreateTable::Column* column : createTable->columns)
    {
        if (isReservedLiteral(column->name))
        {
            hasReservedColName = true;
            break;
        }
    }

    if (hasReservedColName)
    {
        SqliteCreateTable* ctCopy = createTable->typeClone<SqliteCreateTable>();
        ctCopy->table = newName;
        ctCopy->rebuildTokens();
        sqls << ctCopy->detokenize();

        if (doCopyData)
        {
            QStringList colList;
            for (SqliteCreateTable::Column* column : createTable->columns)
                colList << wrapObjIfNeeded(column->name);

            QString cols = colList.join(", ");
            sqls << QString("INSERT INTO %1 (%2) SELECT %2 FROM %3").arg(wrapObjIfNeeded(newName), cols, wrapObjIfNeeded(table));
        }

        sqls << QString("DROP TABLE %1;").arg(wrapObjIfNeeded(table));
        delete ctCopy;
    }
    else
    {
        // No reserved literal as columns. We can use simple way.
        sqls << QString("CREATE TABLE %1 AS SELECT * FROM %2%3;").arg(wrapObjIfNeeded(newName), wrapObjIfNeeded(table), doCopyData ? "" : " LIMIT 0")
             << QString("DROP TABLE %1;").arg(wrapObjIfNeeded(table));
    }

    createTable->table = newName;
    table = newName;
}

QString TableModifier::renameToTemp(bool doCopyData)
{
    QString name = getTempTableName();
    renameTo(name, doCopyData);
    return name;
}

void TableModifier::copyDataTo(const QString& targetTable)
{
    SchemaResolver resolver(db);
    resolver.setIgnoreSystemObjects(true);
    QStringList targetColumns = resolver.getTableColumns(targetTable);
    QStringList colsToCopy;
    for (SqliteCreateTable::Column* column : createTable->columns)
    {
        if (column->hasConstraint(SqliteCreateTable::Column::Constraint::GENERATED))
            continue;

        if (targetColumns.contains(column->name, Qt::CaseInsensitive))
            colsToCopy << wrapObjIfNeeded(column->name);
    }

    copyDataTo(targetTable, colsToCopy, colsToCopy);
}

void TableModifier::handleFks()
{
    tablesHandledForFk << originalTable;

    SchemaResolver resolver(db);
    resolver.setIgnoreSystemObjects(true);
    QStringList fkTables = resolver.getFkReferencingTables(originalTable);

    for (const QString& fkTable : fkTables)
    {
        if (tablesHandledForFk.contains(fkTable, Qt::CaseInsensitive))
            continue; // Avoid recurrent FK handling

        TableModifier subModifier(db, fkTable);
        if (!subModifier.isValid())
        {
            warnings << QObject::tr("Table %1 is referencing table %2, but the foreign key definition will not be updated for new table definition "
                                    "due to problems while parsing DDL of the table %3.").arg(fkTable, originalTable, fkTable);
            continue;
        }

        // Those were removed when fixing #2954. Seem to be useless for subHandleFks(). Unless there's some bug report for it,
        // they should be removed in near future:

        subModifier.usedTempTableNames = usedTempTableNames;
        subModifier.triggerNameToDdlMap = triggerNameToDdlMap;
        subModifier.existingColumns = existingColumns; // for identifying removed columns
        subModifier.tableColMap = tableColMap; // for identifying renamed columns
        subModifier.newName = fkTable;
        subModifier.tablesHandledForFk = tablesHandledForFk;
        subModifier.handleFkAsSubModifier(originalTable, newName);
        sqls += subModifier.generateSqls();
        modifiedTables << fkTable;

        triggerNameToDdlMap = subModifier.triggerNameToDdlMap;
        tablesHandledForFk = subModifier.tablesHandledForFk;
        usedTempTableNames = subModifier.usedTempTableNames;

        modifiedTables += subModifier.getModifiedTables();
        modifiedIndexes += subModifier.getModifiedIndexes();
        modifiedTriggers += subModifier.getModifiedTriggers();
        modifiedViews += subModifier.getModifiedViews();

        warnings += subModifier.getWarnings();
        errors += subModifier.getErrors();
    }
}

void TableModifier::handleFkAsSubModifier(const QString& oldName, const QString& theNewName)
{
    if (!handleFkConstrains(createTable.data(), oldName, theNewName))
        return;

    QString tempName = renameToTemp();

    createTable->table = originalTable;
    createTable->rebuildTokens();
    sqls << createTable->detokenize();

    copyDataTo(originalTable);

    handleFks();

    sqls << QString("DROP TABLE %1;").arg(wrapObjIfNeeded(tempName));

    simpleHandleIndexes();
    simpleHandleTriggers();
}

bool TableModifier::handleFkStmt(SqliteForeignKey* fk, const QString& oldName, const QString& theNewName)
{
    // If table was not renamed (but uses temp table name), we will rename temp name into target name.
    // If table was renamed, we will rename old name to new name.
    bool modified = false;

    // Table
    if (handleName(oldName, theNewName, fk->foreignTable))
        modified = true;

    // Columns
    if (handleIndexedColumns(fk->indexedColumns))
        modified = true;

    return modified;
}

bool TableModifier::handleFkConstrains(SqliteCreateTable* stmt, const QString& oldName, const QString& theNewName)
{
    bool modified = false;
    for (SqliteCreateTable::Constraint*& fk : stmt->getForeignKeysByTable(oldName))
    {
        if (handleFkStmt(fk->foreignKey, oldName, theNewName))
        {
            modified = true;
            if (fk->foreignKey->indexedColumns.isEmpty())
            {
                stmt->constraints.removeOne(fk);
                delete fk;
            }
        }
    }

    for (SqliteCreateTable::Column::Constraint*& fk : stmt->getColumnForeignKeysByTable(oldName))
    {
        if (handleFkStmt(fk->foreignKey, oldName, theNewName))
        {
            modified = true;
            if (fk->foreignKey->indexedColumns.isEmpty())
            {
                stmt->removeColumnConstraint(fk);
                delete fk;
            }
        }
    }
    return modified;
}

bool TableModifier::handleName(const QString& oldName, QString& valueToUpdate)
{
    return handleName(oldName, newName, valueToUpdate);
}

bool TableModifier::handleName(const QString& oldName, const QString& theNewName, QString& valueToUpdate)
{
    if (theNewName.compare(oldName, Qt::CaseInsensitive) == 0)
        return false;

    if (valueToUpdate.compare(oldName, Qt::CaseInsensitive) == 0)
    {
        valueToUpdate = theNewName;
        return true;
    }
    return false;
}

bool TableModifier::handleIndexedColumnsInitial(SqliteOrderBy* col, bool& modified)
{
    if (col->isSimpleColumn())
        return false;

    QString oldExpr = col->expr->tokens.detokenize();
    if (!handleExpr(col->expr))
        qWarning() << "Handling column change in multi-level expression of CREATE INDEX column failed. The change will most probably be skipped in the final update DDL.";

    modified = (col->expr->tokens.detokenize() != oldExpr);
    return true;
}

bool TableModifier::handleIndexedColumnsInitial(SqliteIndexedColumn* col, bool& modified)
{
    UNUSED(col);
    UNUSED(modified);
    return false;
}

bool TableModifier::handleColumnNames(QStringList& columnsToUpdate)
{
    bool modified = false;
    QString lowerName;
    QMutableStringListIterator it(columnsToUpdate);
    while (it.hasNext())
    {
        it.next();

        // If column was modified, assign new name
        lowerName = it.value().toLower();
        if (tableColMap.contains(lowerName))
        {
            it.value() = tableColMap[lowerName];
            modified = true;
            continue;
        }

        // It wasn't modified, but it's not on existing columns list? Remove it.
        if (indexOf(existingColumns, it.value(), Qt::CaseInsensitive) == -1)
        {
            it.remove();
            modified = true;
        }
    }
    return modified;
}

bool TableModifier::handleColumnTokens(TokenList& columnsToUpdate)
{
    bool modified = false;
    QString lowerName;
    QMutableListIterator<TokenPtr> it(columnsToUpdate);
    while (it.hasNext())
    {
        TokenPtr token = it.next();

        // If column was modified, assign new name
        lowerName = stripObjName(token->value).toLower();
        if (tableColMap.contains(lowerName))
        {
            token->value = tableColMap[lowerName];
            modified = true;
            continue;
        }

        // It wasn't modified, but it's not on existing columns list?
        // In case of SELECT it's complicated to remove that token from anywhere
        // in the statement. Replacing it with NULL is a kind of compromise.
        if (indexOf(existingColumns, lowerName, Qt::CaseInsensitive) == -1)
        {
            token->value = "NULL";
            modified = true;
        }
    }
    return modified;
}

bool TableModifier::handleUpdateColumns(SqliteUpdate* update)
{
    bool modified = false;
    QVariant colName;
    QString newName;
    QStringList newNames;
    QMutableListIterator<SqliteUpdate::ColumnAndValue> it(update->keyValueMap);
    while (it.hasNext())
    {
        it.next();

        colName = it.value().first;
        if (colName.userType() == QMetaType::QStringList)
        {
            // List of columns set to a single value
            newNames = handleUpdateColumns(colName.toStringList(), modified);
            if (!modified)
                continue;

            if (newNames.isEmpty())
            {
                it.remove();
                continue;
            }

            // If any column was modified, assign new list
            it.value().first = newNames;
            continue;
        }

        // Single column case
        newName = handleUpdateColumn(colName.toString(), modified);
        if (!modified)
            continue;

        if (newName.isNull())
        {
            it.remove();
            continue;
        }

        // If column was modified, assign new name
        it.value().first = newName;
    }
    return modified;
}

QStringList TableModifier::handleUpdateColumns(const QStringList& colNames, bool& modified)
{
    QStringList newNames;
    for (const QString& colName : colNames)
        newNames << handleUpdateColumn(colName, modified);

    return newNames;
}

QString TableModifier::handleUpdateColumn(const QString& colName, bool& modified)
{
    // If column was modified, assign new name
    QString lowerName = colName.toLower();
    if (tableColMap.contains(lowerName))
    {
        modified = true;
        return tableColMap[lowerName];
    }

    // It wasn't modified, but it's not on existing columns list? Remove it.
    if (indexOf(existingColumns, colName, Qt::CaseInsensitive) == -1)
    {
        modified = true;
        return QString();
    }

    return colName;
}

QStringList TableModifier::getModifiedViews() const
{
    return modifiedViews;
}

bool TableModifier::hasMessages() const
{
    return errors.size() > 0 || warnings.size() > 0;
}

QStringList TableModifier::getModifiedTriggers() const
{
    return modifiedTriggers;
}

QStringList TableModifier::getModifiedIndexes() const
{
    return modifiedIndexes;
}

QStringList TableModifier::getModifiedTables() const
{
    return modifiedTables;
}

QList<SqliteCreateTable::Column*> TableModifier::getColumnsToCopyData(SqliteCreateTablePtr newCreateTable)
{
    QList<SqliteCreateTable::Column*> resultColumns;
    QStringList existingColumnsBefore = createTable->getColumnNames();
    for (SqliteCreateTable::Column*& column : newCreateTable->columns)
    {
        if (column->hasConstraint(SqliteCreateTable::Column::Constraint::GENERATED))
            continue;

        if (!existingColumnsBefore.contains(column->originalName))
            continue; // not copying columns that didn't exist before

        resultColumns << column;
    }
    return resultColumns;
}

void TableModifier::copyDataTo(SqliteCreateTablePtr newCreateTable)
{
    QStringList srcCols;
    QStringList dstCols;
    for (SqliteCreateTable::Column*& column : getColumnsToCopyData(newCreateTable))
    {
        srcCols << wrapObjIfNeeded(column->originalName);
        dstCols << wrapObjIfNeeded(column->name);
    }

    copyDataTo(newCreateTable->table, srcCols, dstCols);
}

void TableModifier::handleIndexes()
{
    SchemaResolver resolver(db);
    resolver.setIgnoreSystemObjects(true);
    QList<SqliteCreateIndexPtr> parsedIndexesForTable = resolver.getParsedIndexesForTable(originalTable);
    for (SqliteCreateIndexPtr& index : parsedIndexesForTable)
        handleIndex(index);
}

void TableModifier::handleIndex(SqliteCreateIndexPtr index)
{
    handleName(originalTable, index->table);
    handleIndexedColumns(index->indexedColumns);
    if (index->indexedColumns.size() > 0)
    {
        index->rebuildTokens();
        sqls << index->detokenize();
        modifiedIndexes << index->index;
    }
    else
    {
        warnings << QObject::tr("All columns indexed by the index %1 are gone. The index will not be recreated after table modification.").arg(index->index);
    }

    // TODO partial index needs handling expr here
}

void TableModifier::handleTriggers()
{
    SchemaResolver resolver(db);
    resolver.setIgnoreSystemObjects(true);
    QList<SqliteCreateTriggerPtr> parsedTriggersForTable = resolver.getParsedTriggersForTable(originalTable, true);
    for (SqliteCreateTriggerPtr& trig : parsedTriggersForTable)
        handleTrigger(trig);
}

void TableModifier::handleTrigger(SqliteCreateTriggerPtr trigger)
{
    // Cloning trigger (to avoid overwritting tokensMap when rebuilding tokens)
    // and determining query string before it's modified by this method.
    SqliteCreateTrigger* triggerClone = dynamic_cast<SqliteCreateTrigger*>(trigger->clone());
    triggerClone->rebuildTokens();
    QString originalQueryString = triggerClone->detokenize();
    delete triggerClone;

    bool forThisTable = (originalTable.compare(trigger->table, Qt::CaseInsensitive) == 0);
    bool alreadyProcessedOnce = modifiedTriggers.contains(trigger->trigger, Qt::CaseInsensitive);

    if (forThisTable)
    {
        // Those routines should run only for trigger targeted for originalTable.
        handleName(originalTable, trigger->table);
        if (trigger->event->type == SqliteCreateTrigger::Event::UPDATE_OF)
            handleColumnNames(trigger->event->columnNames);
    }

    if (alreadyProcessedOnce)
    {
        // The trigger was already modified by handling of some referencing table.
        QString oldDdl = triggerNameToDdlMap[trigger->trigger];
        Parser parser;
        trigger = parser.parse<SqliteCreateTrigger>(oldDdl);
        if (!trigger)
        {
            qCritical() << "Could not parse old (already processed once) trigger. Parser error:" << parser.getErrorString() << ", Old DDL: " << oldDdl;
            warnings << QObject::tr("There is problem with proper processing trigger %1. It may be not fully updated afterwards and will need your attention.")
                        .arg(trigger->trigger);
            return;
        }
    }

    handleTriggerQueries(trigger);

    trigger->rebuildTokens();
    QString newQueryString = trigger->detokenize();
    if (originalQueryString == newQueryString && !forThisTable)
        return; // No query modification was made and trigger is not deleted by this table drop.

    if (trigger->event->type == SqliteCreateTrigger::Event::UPDATE_OF && trigger->event->columnNames.size() == 0)
    {
        warnings << QObject::tr("All columns covered by the trigger %1 are gone. The trigger will not be recreated after table modification.").arg(trigger->trigger);
        return;
    }

    if (alreadyProcessedOnce)
    {
        // We will add new sql to list, at the end, so it's executed after all tables were altered.
        sqls.removeOne(triggerNameToDdlMap[trigger->trigger]);
    }

    if (!forThisTable)
    {
        // If this is for other table, than trigger might be still existing, cause altering this table will not delete trigger.
        sqls << QString("DROP TRIGGER IF EXISTS %1").arg(wrapObjIfNeeded(trigger->trigger));
    }

    sqls << newQueryString;
    modifiedTriggers << trigger->trigger;
    triggerNameToDdlMap[trigger->trigger] = newQueryString;
}

void TableModifier::handleTriggerQueries(SqliteCreateTriggerPtr trigger)
{
    SqliteQuery* newQuery = nullptr;
    QList<SqliteQuery*> newQueries;
    for (SqliteQuery*& query : trigger->queries)
    {
        // The handleTriggerQuery() may delete the input query object. Don't refer to it later.
        newQuery = handleTriggerQuery(query, trigger->trigger, trigger->table);
        if (newQuery)
            newQueries << newQuery;
        else
            errors << QObject::tr("Cannot not update trigger %1 according to table %2 modification.").arg(trigger->trigger, originalTable);
    }
    trigger->queries = newQueries;
}

void TableModifier::handleViews()
{
    SchemaResolver resolver(db);
    resolver.setIgnoreSystemObjects(true);
    QList<SqliteCreateViewPtr> parsedViewsForTable = resolver.getParsedViewsForTable(originalTable);
    for (SqliteCreateViewPtr& view : parsedViewsForTable)
        handleView(view);
}

void TableModifier::handleView(SqliteCreateViewPtr view)
{
    SqliteSelect* newSelect = handleSelect(view->select);
    if (!newSelect)
    {
        errors << QObject::tr("Cannot not update view %1 according to table %2 modifications.\nThe view will remain as it is.").arg(view->view, originalTable);
        return;
    }

    view->select->rebuildTokens();
    QString originalSelect = view->select->tokens.detokenize();

    newSelect->rebuildTokens();
    QString newSelectStr = newSelect->tokens.detokenize();

    if (originalSelect == newSelectStr)
        return;

    delete view->select;
    view->select = newSelect;
    view->select->setParent(view.data());
    view->rebuildTokens();

    sqls << QString("DROP VIEW %1;").arg(wrapObjIfNeeded(view->view));
    sqls << view->detokenize();

    simpleHandleTriggers(view->view);

    modifiedViews << view->view;
}

SqliteQuery* TableModifier::handleTriggerQuery(SqliteQuery* query, const QString& trigName, const QString& trigTable)
{
    SqliteSelect* select = dynamic_cast<SqliteSelect*>(query);
    if (select)
        return handleSelect(select, trigTable);

    SqliteUpdate* update = dynamic_cast<SqliteUpdate*>(query);
    if (update)
        return handleTriggerUpdate(update, trigName, trigTable);

    SqliteInsert* insert = dynamic_cast<SqliteInsert*>(query);
    if (insert)
        return handleTriggerInsert(insert, trigName, trigTable);

    SqliteDelete* del = dynamic_cast<SqliteDelete*>(query);
    if (del)
        return handleTriggerDelete(del, trigName, trigTable);

    return nullptr;
}

SqliteSelect* TableModifier::handleSelect(SqliteSelect* select, const QString& trigTable)
{
    SelectResolver selectResolver(db, select->detokenize());

    // Table name
    QList<SqliteSelect::Core::SingleSource*> selSources = select->getAllTypedStatements<SqliteSelect::Core::SingleSource>();
    TokenList tableTokens;
    StrHash<SelectResolver::Table> resolvedTables;
    for (SqliteSelect::Core*& core : select->coreSelects)
    {
        resolvedTables = tablesAsNameHash(selectResolver.resolveTables(core));

        tableTokens = core->getContextTableTokens(false);
        for (TokenPtr& token : tableTokens)
        {
            if (token->value.compare(originalTable, Qt::CaseInsensitive) != 0)
                continue;

            // Check if that table name is the same as its alias name, so we use alias name and we don't rename it here, cause it's alias, not table
            if (isTableAliasUsedForColumn(token, resolvedTables, selSources))
                continue;

            token->value = newName;
        }
    }

    // Column names
    TokenList columnTokens = select->getContextColumnTokens(false);
    QList<SelectResolver::Column> columns = selectResolver.translateToColumns(select, columnTokens);

    TokenList columnTokensToChange;
    for (int i = 0; i < columnTokens.size(); i++)
    {
        if (columns[i].type != SelectResolver::Column::COLUMN)
            continue;

        if (originalTable.compare(columns[i].table, Qt::CaseInsensitive) == 0)
            columnTokensToChange << columnTokens[i];
    }

    handleColumnTokens(columnTokensToChange);

    // Rebuilding modified tokens into the select object
    QString selectSql = select->detokenize();
    SqliteQueryPtr queryPtr = parseQuery(selectSql);
    if (!queryPtr)
    {
        qCritical() << "Could not parse modified SELECT in TableModifier::handleSelect().";
        return nullptr;
    }

    SqliteSelectPtr selectPtr = queryPtr.dynamicCast<SqliteSelect>();
    if (!selectPtr)
    {
        qCritical() << "Could cast into SELECT in TableModifier::handleSelect().";
        return nullptr;
    }

    if (!trigTable.isNull() && !handleAllExprWithTrigTable(selectPtr.data(), trigTable))
        return nullptr;

    return new SqliteSelect(*selectPtr.data());
}

StrHash<SelectResolver::Table> TableModifier::tablesAsNameHash(const QSet<SelectResolver::Table>& resolvedTables)
{
    StrHash<SelectResolver::Table> result;
    for (const SelectResolver::Table& tab : resolvedTables)
        result[tab.table] = tab;

    return result;
}

bool TableModifier::isTableAliasUsedForColumn(const TokenPtr &token, const StrHash<SelectResolver::Table> &resolvedTables, const QList<SqliteSelect::Core::SingleSource *> &selSources)
{
    // If we don't have the table token on the list of resolved select tables, we don't consider it as aliased
    if (!resolvedTables.contains(token->value, Qt::CaseInsensitive))
    {
        qWarning() << "Table" << token->value << "in table tokens processed by TableModifier, but not in resolved SELECT tables.";
        return false;
    }

    SelectResolver::Table table = resolvedTables.value(token->value, Qt::CaseInsensitive);
    if (table.tableAlias.isNull())
        return false;

    if (table.tableAlias.compare(token->value, Qt::CaseInsensitive) != 0)
        return false;

    // If the table token is mentioned in FROM clause, it's not a subject for aliased usage, cuase it defines alias, not uses it.
    for (SqliteSelect::Core::SingleSource* src : selSources)
    {
        if (src->tokens.contains(token))
            return false;
    }

    return true;
}

SqliteUpdate* TableModifier::handleTriggerUpdate(SqliteUpdate* update, const QString& trigName, const QString& trigTable)
{
    if (update->table.compare(originalTable, Qt::CaseInsensitive) == 0)
    {
        // Table name
        update->table = newName;

        // Column names
        handleUpdateColumns(update);
    }

    // Any embedded selects
    bool embedSelectsOk = handleSubSelects(update, trigTable);
    bool embedExprOk = handleAllExprWithTrigTable(update, trigTable);
    if (!embedSelectsOk || !embedExprOk)
    {
        warnings << QObject::tr("There is a problem with updating an %1 statement within %2 trigger. "
                                "One of the %1 substatements which might be referring to table %3 cannot be properly modified. "
                                "Manual update of the trigger may be necessary.").arg("UPDATE", trigName, originalTable);
    }

    return update;
}

SqliteInsert* TableModifier::handleTriggerInsert(SqliteInsert* insert, const QString& trigName, const QString& trigTable)
{
    if (insert->table.compare(originalTable, Qt::CaseInsensitive) == 0)
    {
        // Table name
        insert->table = newName;

        // Column names
        handleColumnNames(insert->columnNames);
    }

    // Any embedded selects
    bool embedSelectsOk = handleSubSelects(insert, trigTable);
    bool embedExprOk = handleAllExprWithTrigTable(insert, trigTable);
    if (!embedSelectsOk || !embedExprOk)
    {
        warnings << QObject::tr("There is a problem with updating an %1 statement within %2 trigger. "
                                "One of the %1 substatements which might be referring to table %3 cannot be properly modified. "
                                "Manual update of the trigger may be necessary.").arg("INSERT", trigName, originalTable);
    }

    return insert;
}

SqliteDelete* TableModifier::handleTriggerDelete(SqliteDelete* del, const QString& trigName, const QString& trigTable)
{
    // Table name
    if (del->table.compare(originalTable, Qt::CaseInsensitive) == 0)
        del->table = newName;

    // Any embedded selects
    bool embedSelectsOk = handleSubSelects(del, trigTable);
    bool embedExprOk = handleAllExprWithTrigTable(del, trigTable);
    if (!embedSelectsOk || !embedExprOk)
    {
        warnings << QObject::tr("There is a problem with updating an %1 statement within %2 trigger. "
                                "One of the %1 substatements which might be referring to table %3 cannot be properly modified. "
                                "Manual update of the trigger may be necessary.").arg("DELETE", trigName, originalTable);
    }

    return del;
}

bool TableModifier::handleSubSelects(SqliteStatement* stmt, const QString& trigTable)
{
    bool embedSelectsOk = true;
    QList<SqliteSelect*> selects = stmt->getAllTypedStatements<SqliteSelect>();
    SqliteExpr* expr = nullptr;
    for (SqliteSelect* select : selects)
    {
        if (select->coreSelects.size() >= 1 && select->coreSelects.first()->valuesMode)
        {
            // INSERT with VALUES() as subselect
            continue;
        }

        expr = dynamic_cast<SqliteExpr*>(select->parentStatement());
        if (!expr)
        {
            embedSelectsOk = false;
            continue;
        }

        if (!handleExprWithSelect(expr, trigTable))
            embedSelectsOk = false;
    }
    return embedSelectsOk;
}

bool TableModifier::handleExprWithSelect(SqliteExpr* expr, const QString& trigTable)
{
    if (!expr->select)
    {
        qCritical() << "No SELECT in TableModifier::handleExprWithSelect()";
        return false;
    }

    SqliteSelect* newSelect = handleSelect(expr->select, trigTable);
    if (!newSelect)
    {
        qCritical() << "Could not generate new SELECT in TableModifier::handleExprWithSelect()";
        return false;
    }

    delete expr->select;
    expr->select = newSelect;
    expr->select->setParent(expr);
    return true;
}

bool TableModifier::handleAllExprWithTrigTable(SqliteStatement* stmt, const QString& contextTable)
{
    if (contextTable != originalTable)
        return true;

    return handleExprListWithTrigTable(stmt->getAllTypedStatements<SqliteExpr>());
}

bool TableModifier::handleExprListWithTrigTable(const QList<SqliteExpr*>& exprList)
{
    for (SqliteExpr* expr : exprList)
    {
        if (!handleExprWithTrigTable(expr))
            return false;
    }
    return true;
}

bool TableModifier::handleExprWithTrigTable(SqliteExpr* expr)
{
    if (expr->mode != SqliteExpr::Mode::ID)
        return true;

    if (!expr->database.isNull())
        return true;

    if (expr->table.compare("old", Qt::CaseInsensitive) != 0 && expr->table.compare("new", Qt::CaseInsensitive) != 0)
        return true;

    QStringList columns = QStringList({expr->column});
    if (!handleColumnNames(columns))
        return true;

    if (columns.isEmpty())
    {
        qDebug() << "Column in the expression is no longer present in the table. Cannot update the expression automatically.";
        return false;
    }

    expr->column = columns.first();
    return true;
}

bool TableModifier::handleExpr(SqliteExpr* expr)
{
    // Handle subqueries
    QList<SqliteExpr*> exprList;
    exprList << expr->expr1;
    exprList << expr->expr2;
    exprList << expr->expr3;
    exprList.append(expr->exprList);
    exprList.removeAll(nullptr);
    if (!exprList.isEmpty())
    {
        bool res = true;
        for (SqliteExpr*& e : exprList)
        {
            res &= handleExpr(e);
            if (!res)
                break;
        }
        return res;
    }

    // No need to handle subselect. Currently handleExpr() is used only in context of expr index column (in CREATE INDEX),
    // which does not allow subselects. If the method comes to use with subselects supported, then this has to be implemented.

    // Handle specific column
    if (expr->mode != SqliteExpr::Mode::ID)
        return true;

    if (!expr->database.isNull())
        return true;

    QStringList columns = QStringList({expr->column});
    if (!handleColumnNames(columns))
        return true;

    if (columns.isEmpty())
    {
        qDebug() << "Column in the expression is no longer present in the table. Cannot update the expression automatically.";
        return false;
    }

    expr->column = columns.first();
    return true;
}

void TableModifier::simpleHandleIndexes()
{
    SchemaResolver resolver(db);
    resolver.setIgnoreSystemObjects(true);
    sqls += resolver.getIndexDdlsForTable(originalTable);
}

void TableModifier::simpleHandleTriggers(const QString& view)
{
    SchemaResolver resolver(db);
    resolver.setIgnoreSystemObjects(true);
    sqls += resolver.getTriggerDdlsForTableOrView(view.isNull() ? originalTable : view);
}

SqliteQueryPtr TableModifier::parseQuery(const QString& ddl)
{
    Parser parser;
    if (!parser.parse(ddl) || parser.getQueries().size() == 0)
        return SqliteQueryPtr();

    return parser.getQueries().first();
}

void TableModifier::copyDataTo(const QString& targetTable, const QStringList& srcCols, const QStringList& dstCols)
{
    sqls << QString("INSERT INTO %1 (%2) SELECT %3 FROM %4;").arg(wrapObjIfNeeded(targetTable), dstCols.join(", "), srcCols.join(", "),
                                                                 wrapObjIfNeeded(table));
}

QStringList TableModifier::generateSqls() const
{
    return sqls;
}

bool TableModifier::isValid() const
{
    return !createTable.isNull();
}

QStringList TableModifier::getErrors() const
{
    return errors;
}

QStringList TableModifier::getWarnings() const
{
    return warnings;
}

void TableModifier::init()
{
    originalTable = table;
    parseDdl();
}

void TableModifier::parseDdl()
{
    SchemaResolver resolver(db);
    resolver.setIgnoreSystemObjects(true);
    QString ddl = resolver.getObjectDdl(database, table, SchemaResolver::TABLE);
    if (ddl.isNull())
    {
        qCritical() << "Could not find object's ddl while parsing table ddl in the TableModifier.";
        return;
    }

    Parser parser;
    if (!parser.parse(ddl))
    {
        qCritical() << "Could not parse table's' ddl in the TableModifier. The ddl is:" << ddl;
        return;
    }

    if (parser.getQueries().size() != 1)
    {
        qCritical() << "Parsed ddl produced more or less than 1 query in the TableModifier. The ddl is:" << ddl;
        return;
    }

    SqliteQueryPtr query = parser.getQueries().first();
    SqliteCreateTablePtr createTable = query.dynamicCast<SqliteCreateTable>();
    if (!createTable)
    {
        qCritical() << "Parsed ddl produced something else than CreateTable statement in the TableModifier. The ddl is:" << ddl;
        return;
    }

    this->createTable = createTable;
}

QString TableModifier::getTempTableName()
{
    SchemaResolver resolver(db);
    resolver.setIgnoreSystemObjects(true);
    QString name = resolver.getUniqueName("sqlitestudio_temp_table", usedTempTableNames);
    usedTempTableNames << name;
    return name;
}
