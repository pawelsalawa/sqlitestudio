#include "tablemodifier.h"
#include "common/utils_sql.h"
#include "parser/parser.h"
#include "schemaresolver.h"
#include "selectresolver.h"
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

    QString tempTableName;
    if (table.compare(newName, Qt::CaseInsensitive) == 0)
        tempTableName = renameToTemp();

    newCreateTable->rebuildTokens();
    sqls << newCreateTable->detokenize();
    copyDataTo(newCreateTable);

    handleFks(tempTableName);

    // If temp table was created, it means that table name hasn't changed. In that case we need to cleanup temp table (drop it).
    // Otherwise, the table name has changed, therefor there still remains the old table which we copied data from - we need to drop it here.
    sqls << QString("DROP TABLE %1;").arg(wrapObjIfNeeded(tempTableName.isNull() ? originalTable : tempTableName, dialect));

    handleIndexes();
    handleTriggers();
    handleViews();
}

void TableModifier::renameTo(const QString& newName)
{
    if (!createTable)
        return;

    if (dialect == Dialect::Sqlite3)
    {
        sqls << QString("ALTER TABLE %1 RENAME TO %2;").arg(wrapObjIfNeeded(table, dialect), wrapObjIfNeeded(newName, dialect));
    }
    else
    {
        sqls << QString("CREATE TABLE %1 AS SELECT * FROM %2;").arg(wrapObjIfNeeded(newName, dialect), wrapObjIfNeeded(table, dialect))
             << QString("DROP TABLE %1;").arg(wrapObjIfNeeded(table, dialect));
    }

    table = newName;
    createTable->table = newName;
}

QString TableModifier::renameToTemp()
{
    QString name = getTempTableName();
    renameTo(name);
    return name;
}

void TableModifier::copyDataTo(const QString& targetTable)
{
    SchemaResolver resolver(db);
    QStringList targetColumns = resolver.getTableColumns(targetTable);
    QStringList colsToCopy;
    foreach (SqliteCreateTable::Column* column, createTable->columns)
        if (targetColumns.contains(column->name, Qt::CaseInsensitive))
            colsToCopy << wrapObjIfNeeded(column->name, dialect);

    copyDataTo(targetTable, colsToCopy, colsToCopy);
}

void TableModifier::handleFks(const QString& tempTableName)
{
    SchemaResolver resolver(db);

    QStringList fkTables = resolver.getFkReferencingTables(originalTable);

    foreach (const QString& fkTable, fkTables)
    {
        TableModifier subModifier(db, fkTable);
        if (!subModifier.isValid())
        {
            warnings << QObject::tr("Table %1 is referencing table %2, but the foreign key definition will not be updated for new table definition "
                                    "due to problems while parsing DDL of the table %3.").arg(fkTable, originalTable, fkTable);
            continue;
        }

        subModifier.usedTempTableNames = usedTempTableNames;
        subModifier.tableColMap = tableColMap;
        subModifier.triggerNameToDdlMap = triggerNameToDdlMap;
        subModifier.existingColumns = existingColumns;
        subModifier.newName = newName;
        subModifier.subHandleFks(originalTable, tempTableName);
        sqls += subModifier.generateSqls();
        modifiedTables << fkTable;
        triggerNameToDdlMap = subModifier.triggerNameToDdlMap;

        modifiedTables += subModifier.getModifiedTables();
        modifiedIndexes += subModifier.getModifiedIndexes();
        modifiedTriggers += subModifier.getModifiedTriggers();
        modifiedViews += subModifier.getModifiedViews();

        warnings += subModifier.getWarnings();
        errors += subModifier.getErrors();
    }
}

void TableModifier::subHandleFks(const QString& oldName, const QString& oldTempName)
{
    bool modified = false;
    foreach (SqliteCreateTable::Constraint* fk, createTable->getForeignKeysByTable(oldName))
    {
        if (subHandleFks(fk->foreignKey, oldName, oldTempName))
            modified = true;
    }

    foreach (SqliteCreateTable::Column::Constraint* fk, createTable->getColumnForeignKeysByTable(oldName))
    {
        if (subHandleFks(fk->foreignKey, oldName, oldTempName))
            modified = true;
    }

    if (!modified)
        return;

    QString tempName = renameToTemp();

    createTable->table = originalTable;
    createTable->rebuildTokens();
    sqls << createTable->detokenize();

    copyDataTo(originalTable);

    handleFks(tempName);

    sqls << QString("DROP TABLE %1;").arg(wrapObjIfNeeded(tempName, dialect));

    simpleHandleIndexes();
    simpleHandleTriggers();
}

bool TableModifier::subHandleFks(SqliteForeignKey* fk, const QString& oldName, const QString& oldTempName)
{
    // If table was not renamed (but uses temp table name), we will rename temp name into target name.
    // If table was renamed, we will rename old name to new name.
    bool modified = false;

    // Table
    if (handleName(oldName, fk->foreignTable))
        modified = true;
    else if (!oldTempName.isNull() && fk->foreignTable.compare(oldName, Qt::CaseInsensitive) == 0)
    {
        // This is the case when main table was not renamed - we will stay with the original name, but we need to mark it as modified,
        // so the table gets recreated (the temp table is changed to the original table name in this FK).
        modified = true;
    }

    // Columns
    if (handleIndexedColumns(fk->indexedColumns))
        modified = true;

    return modified;
}

bool TableModifier::handleName(const QString& oldName, QString& valueToUpdate)
{
    if (newName.compare(oldName, Qt::CaseInsensitive) == 0)
        return false;

    if (valueToUpdate.compare(oldName, Qt::CaseInsensitive) == 0)
    {
        valueToUpdate = newName;
        return true;
    }
    return false;
}

bool TableModifier::handleIndexedColumns(QList<SqliteIndexedColumn*>& columnsToUpdate)
{
    bool modified = false;
    QString lowerName;
    QMutableListIterator<SqliteIndexedColumn*> it(columnsToUpdate);
    while (it.hasNext())
    {
        SqliteIndexedColumn* idxCol = it.next();

        // If column was modified, assign new name
        lowerName = idxCol->name.toLower();
        if (tableColMap.contains(lowerName))
        {
            idxCol->name = tableColMap[lowerName];
            modified = true;
            continue;
        }

        // It wasn't modified, but it's not on existing columns list? Remove it.
        if (indexOf(existingColumns, idxCol->name, Qt::CaseInsensitive) == -1)
        {
            it.remove();
            modified = true;
        }
    }
    return modified;
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
        lowerName = token->value.toLower();
        if (tableColMap.contains(lowerName))
        {
            token->value = tableColMap[lowerName];
            modified = true;
            continue;
        }

        // It wasn't modified, but it's not on existing columns list?
        // In case of SELECT it's complicated to remove that token from anywhere
        // in the statement. Replacing it with NULL is a kind of compromise.
        if (indexOf(existingColumns, token->value, Qt::CaseInsensitive) == -1)
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
    QString lowerName;
    QMutableListIterator<SqliteUpdate::ColumnAndValue> it(update->keyValueMap);
    while (it.hasNext())
    {
        it.next();

        // If column was modified, assign new name
        lowerName = it.value().first.toLower();
        if (tableColMap.contains(lowerName))
        {
            it.value().first = tableColMap[lowerName];
            modified = true;
            continue;
        }

        // It wasn't modified, but it's not on existing columns list? Remove it.
        if (indexOf(existingColumns, it.value().first, Qt::CaseInsensitive) == -1)
        {
            it.remove();
            modified = true;
        }
    }
    return modified;
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

void TableModifier::copyDataTo(SqliteCreateTablePtr newCreateTable)
{
    QStringList existingColumns = createTable->getColumnNames();

    QStringList srcCols;
    QStringList dstCols;
    foreach (SqliteCreateTable::Column* column, newCreateTable->columns)
    {
        if (!existingColumns.contains(column->originalName))
            continue; // not copying columns that didn't exist before

        srcCols << wrapObjIfNeeded(column->originalName, dialect);
        dstCols << wrapObjIfNeeded(column->name, dialect);
    }

    copyDataTo(newCreateTable->table, srcCols, dstCols);
}

void TableModifier::handleIndexes()
{
    SchemaResolver resolver(db);
    QList<SqliteCreateIndexPtr> parsedIndexesForTable = resolver.getParsedIndexesForTable(originalTable);
    foreach (SqliteCreateIndexPtr index, parsedIndexesForTable)
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
    QList<SqliteCreateTriggerPtr> parsedTriggersForTable = resolver.getParsedTriggersForTable(originalTable, true);
    foreach (SqliteCreateTriggerPtr trig, parsedTriggersForTable)
        handleTrigger(trig);
}

void TableModifier::handleTrigger(SqliteCreateTriggerPtr trigger)
{
    trigger->rebuildTokens();
    QString originalQueryString = trigger->detokenize();

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
        Parser parser(dialect);
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
        sqls << QString("DROP TRIGGER IF EXISTS %1").arg(wrapObjIfNeeded(trigger->trigger, dialect));
    }

    sqls << newQueryString;
    modifiedTriggers << trigger->trigger;
    triggerNameToDdlMap[trigger->trigger] = newQueryString;
}

void TableModifier::handleTriggerQueries(SqliteCreateTriggerPtr trigger)
{
    SqliteQuery* newQuery = nullptr;
    QList<SqliteQuery*> newQueries;
    foreach (SqliteQuery* query, trigger->queries)
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
    QList<SqliteCreateViewPtr> parsedViewsForTable = resolver.getParsedViewsForTable(originalTable);
    foreach (SqliteCreateViewPtr view, parsedViewsForTable)
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

    delete view->select;
    view->select = newSelect;
    view->select->setParent(view.data());
    view->rebuildTokens();

    sqls << QString("DROP VIEW %1;").arg(wrapObjIfNeeded(view->view, dialect));
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
    // Table name
    TokenList tableTokens = select->getContextTableTokens(false);
    foreach (TokenPtr token, tableTokens)
    {
        if (token->value.compare(originalTable, Qt::CaseInsensitive) == 0)
            token->value = newName;
    }

    // Column names
    TokenList columnTokens = select->getContextColumnTokens(false);
    SelectResolver selectResolver(db, select->detokenize());
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
    foreach (SqliteSelect* select, selects)
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

void TableModifier::simpleHandleIndexes()
{
    SchemaResolver resolver(db);
    QList<SqliteCreateIndexPtr> parsedIndexesForTable = resolver.getParsedIndexesForTable(originalTable);
    foreach (SqliteCreateIndexPtr index, parsedIndexesForTable)
        sqls << index->detokenize();
}

void TableModifier::simpleHandleTriggers(const QString& view)
{
    SchemaResolver resolver(db);
    QList<SqliteCreateTriggerPtr> parsedTriggers ;
    if (!view.isNull())
        parsedTriggers = resolver.getParsedTriggersForView(view);
    else
        parsedTriggers = resolver.getParsedTriggersForTable(originalTable);

    foreach (SqliteCreateTriggerPtr trig, parsedTriggers)
        sqls << trig->detokenize();
}

SqliteQueryPtr TableModifier::parseQuery(const QString& ddl)
{
    Parser parser(dialect);
    if (!parser.parse(ddl) || parser.getQueries().size() == 0)
        return SqliteQueryPtr();

    return parser.getQueries().first();
}

void TableModifier::copyDataTo(const QString& targetTable, const QStringList& srcCols, const QStringList& dstCols)
{
    sqls << QString("INSERT INTO %1 (%2) SELECT %3 FROM %4;").arg(wrapObjIfNeeded(targetTable, dialect), dstCols.join(", "), srcCols.join(", "),
                                                                 wrapObjIfNeeded(table, dialect));
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
    dialect = db->getDialect();
    originalTable = table;
    parseDdl();
}

void TableModifier::parseDdl()
{
    SchemaResolver resolver(db);
    QString ddl = resolver.getObjectDdl(database, table, SchemaResolver::TABLE);
    if (ddl.isNull())
    {
        qCritical() << "Could not find object's ddl while parsing table ddl in the TableModifier.";
        return;
    }

    Parser parser(dialect);
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
    QString name = resolver.getUniqueName("sqlitestudio_temp_table", usedTempTableNames);
    usedTempTableNames << name;
    return name;
}
