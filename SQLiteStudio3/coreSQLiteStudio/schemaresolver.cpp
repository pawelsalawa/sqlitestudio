#include "schemaresolver.h"
#include "db/db.h"
#include "common/utils_sql.h"
#include "parser/parsererror.h"
#include "parser/ast/sqlitecreatetable.h"
#include "parser/ast/sqlitecreateindex.h"
#include "parser/ast/sqlitecreatetrigger.h"
#include "parser/ast/sqlitecreateview.h"
#include "parser/ast/sqlitecreatevirtualtable.h"
#include "parser/ast/sqlitetablerelatedddl.h"
#include <QDebug>

const char* sqliteMasterDdl =
    "CREATE TABLE sqlite_master (type text, name text, tbl_name text, rootpage integer, sql text)";
const char* sqliteTempMasterDdl =
    "CREATE TABLE sqlite_temp_master (type text, name text, tbl_name text, rootpage integer, sql text)";

SchemaResolver::SchemaResolver(Db *db)
    : db(db)
{
    parser = new Parser(db->getDialect());
}

SchemaResolver::~SchemaResolver()
{
    delete parser;
}

QStringList SchemaResolver::getTables(const QString &database)
{
    QStringList tables = getObjects(database, "table");
    if (!ignoreSystemObjects)
        tables << "sqlite_master" << "sqlite_temp_master";

    return tables;
}

QStringList SchemaResolver::getIndexes(const QString &database)
{
    QStringList indexes = getObjects(database, "index");
    if (ignoreSystemObjects)
        filterSystemIndexes(indexes);

    return indexes;
}

QStringList SchemaResolver::getTriggers(const QString &database)
{
    return getObjects(database, "trigger");
}

QStringList SchemaResolver::getViews(const QString &database)
{
    return getObjects(database, "view");
}

QMap<QString, QStringList> SchemaResolver::getGroupedIndexes(const QString &database)
{
    QStringList allIndexes = getIndexes(database);
    return getGroupedObjects(database, allIndexes, SqliteQueryType::CreateIndex);
}

QMap<QString, QStringList> SchemaResolver::getGroupedTriggers(const QString &database)
{
    QStringList allTriggers = getTriggers(database);
    return getGroupedObjects(database, allTriggers, SqliteQueryType::CreateTrigger);
}

QMap<QString, QStringList> SchemaResolver::getGroupedObjects(const QString &database, const QStringList &inputList, SqliteQueryType type)
{
    QString strType = sqliteQueryTypeToString(type);
    QMap<QString, QStringList> groupedTriggers;

    SqliteQueryPtr parsedQuery;
    SqliteTableRelatedDdlPtr tableRelatedDdl;

    foreach (QString object, inputList)
    {
        parsedQuery = getParsedObject(database, object);
        if (!parsedQuery)
        {
            qWarning() << "Could not get parsed object for " << strType << ":" << object;
            continue;
        }

        tableRelatedDdl  = parsedQuery.dynamicCast<SqliteTableRelatedDdl>();
        if (!tableRelatedDdl)
        {
            qWarning() << "Parsed object is not of expected type. Expected" << strType
                       << ", but got" << sqliteQueryTypeToString(parsedQuery->queryType);
            continue;
        }

        groupedTriggers[tableRelatedDdl->getTargetTable()] << object;
    }

    return groupedTriggers;
}

bool SchemaResolver::isFilteredOut(const QString& value, const QString& type)
{
    if (ignoreSystemObjects)
    {
        if (type == "table" && isSystemTable(value))
            return true;

        if (type == "index" && isSystemIndex(value, db->getDialect()))
            return true;
    }

    return false;
}

QSet<QString> SchemaResolver::getDatabases()
{
    return db->getAllAttaches();
}

QStringList SchemaResolver::getTableColumns(const QString& table)
{
    return getTableColumns("main", table);
}

QStringList SchemaResolver::getTableColumns(const QString &database, const QString &table)
{
    QStringList columns; // result

    SqliteQueryPtr query = getParsedObject(database, table);
    if (!query)
        return columns;

    SqliteCreateTablePtr createTable = query.dynamicCast<SqliteCreateTable>();
    SqliteCreateVirtualTablePtr createVirtualTable = query.dynamicCast<SqliteCreateVirtualTable>();
    if (!createTable && !createVirtualTable)
    {
        qDebug() << "Parsed DDL is neither a CREATE TABLE or CREATE VIRTUAL TABLE statement. It's: "
                 << sqliteQueryTypeToString(query->queryType);

        return columns;
    }

    // If we parsed virtual table, then we have to create temporary regular table to extract columns.
    if (createVirtualTable)
    {
        createTable = virtualTableAsRegularTable(database, table);
        if (!createTable)
            return columns;
    }

    // Now we have a regular table, let's extract columns.
    foreach (SqliteCreateTable::Column* column, createTable->columns)
        columns << column->name;

    return columns;
}

QHash<QString, QStringList> SchemaResolver::getAllTableColumns(const QString &database)
{
    QHash<QString, QStringList> tableColumns;
    foreach (QString table, getTables(database))
        tableColumns[table] = getTableColumns(database, table);

    return tableColumns;
}

QStringList SchemaResolver::getViewColumns(const QString& view)
{
    return getViewColumns("main", view);
}

QStringList SchemaResolver::getViewColumns(const QString& database, const QString& view)
{
    QList<SelectResolver::Column> resolvedColumns = getViewColumnObjects(database, view);
    QStringList columns;
    foreach (const SelectResolver::Column& col, resolvedColumns)
        columns << col.displayName;

    return columns;
}

QList<SelectResolver::Column> SchemaResolver::getViewColumnObjects(const QString& view)
{
    return getViewColumnObjects("main", view);
}

QList<SelectResolver::Column> SchemaResolver::getViewColumnObjects(const QString& database, const QString& view)
{
    QList<SelectResolver::Column> results;
    SqliteQueryPtr query = getParsedObject(database, view);
    if (!query)
        return results;

    SqliteCreateViewPtr createView = query.dynamicCast<SqliteCreateView>();
    if (!createView)
    {
        qDebug() << "Parsed query is not CREATE VIEW statement as expected.";
        return results;
    }

    SelectResolver resolver(db, createView->select->detokenize());
    QList<QList<SelectResolver::Column> > resolvedColumns = resolver.resolve(createView->select);
    if (resolvedColumns.size() == 0)
    {
        qDebug() << "Could not resolve any results column from the view object.";
        return results;
    }
    return resolvedColumns.first();
}

SqliteCreateTablePtr SchemaResolver::virtualTableAsRegularTable(const QString &database, const QString &table)
{
    Dialect dialect = db->getDialect();
    QString strippedName = stripObjName(table, dialect);
    QString dbName = getPrefixDb(database, dialect);

    // Create temp table to see columns.
    QString newTable = db->getUniqueNewObjectName(strippedName);
    QString origTable = wrapObjName(strippedName, dialect);
    db->exec(QString("CREATE TEMP TABLE %1 AS SELECT * FROM %2.%3 LIMIT 0;").arg(newTable, dbName, origTable));

    // Get parsed DDL of the temp table.
    SqliteQueryPtr query = getParsedObject("temp", newTable);
    if (!query)
        return SqliteCreateTablePtr();

    SqliteCreateTablePtr createTable = query.dynamicCast<SqliteCreateTable>();

    // Getting rid of the temp table.
    db->exec(QString("DROP TABLE %1;").arg(newTable));

    // Returning results. Might be null.
    return createTable;
}

QString SchemaResolver::getObjectDdl(const QString& name)
{
    return getObjectDdl("main", name);
}

QString SchemaResolver::getObjectDdl(const QString &database, const QString &name)
{
    if (name.isNull())
        return QString::null;

    Dialect dialect = db->getDialect();
    // In case of sqlite_master or sqlite_temp_master we have static definitions
    QString lowerName = stripObjName(name, dialect).toLower();
    if (lowerName == "sqlite_master")
        return getSqliteMasterDdl(false);
    else if (lowerName == "sqlite_temp_master")
        return getSqliteMasterDdl(true);

    // Prepare db prefix.
    QString dbName = getPrefixDb(database, dialect);

    // Get the DDL
    QVariant results = db->exec(QString(
                "SELECT sql FROM %1.sqlite_master WHERE lower(name) = '%2';").arg(dbName, escapeString(lowerName))
            )->getSingleCell();

    // Validate query results
    if (!results.isValid() || results.isNull())
    {
        qDebug() << "Could not get object's DDL:" << database << "." << name;
        return QString::null;
    }

    // The DDL string
    QString resStr = results.toString();

    // If the DDL doesn't have semicolon at the end (usually the case), add it.
    if (!resStr.trimmed().endsWith(";"))
        resStr += ";";

    // Return the DDL
    return resStr;
}

SqliteQueryPtr SchemaResolver::getParsedObject(const QString &name)
{
    return getParsedObject("main", name);
}

SqliteQueryPtr SchemaResolver::getParsedObject(const QString &database, const QString &name)
{
    // Get DDL
    QString ddl = getObjectDdl(database, name);
    if (ddl.isNull())
        return SqliteQueryPtr();

    // Parse DDL
    if (!parser->parse(ddl))
    {
        qDebug() << "Could not parse DDL for parsing object by SchemaResolver. Errors are:";
        foreach (ParserError* err, parser->getErrors())
            qDebug() << err->getMessage();

        return SqliteQueryPtr();
    }

    // Validate parsed DDL
    QList<SqliteQueryPtr> queries = parser->getQueries();
    if (queries.size() == 0)
    {
        qDebug() << "No parsed query while getting temp table columns.";
        return SqliteQueryPtr();
    }

    // Preparing results
    return queries[0];
}

QStringList SchemaResolver::getObjects(const QString &type)
{
    return getObjects(QString::null, type);
}

QStringList SchemaResolver::getObjects(const QString &database, const QString &type)
{
    QStringList resList;
    QString dbName = getPrefixDb(database, db->getDialect());

    SqlResultsPtr results = db->exec(QString(
                "SELECT name FROM %1.sqlite_master WHERE type = '%2';").arg(dbName, type));

    QString value;
    foreach (SqlResultsRowPtr row, results->getAll())
    {
        value = row->value(0).toString();
        if (!isFilteredOut(value, type))
            resList << value;
    }

    return resList;
}

QStringList SchemaResolver::getAllObjects()
{
    return getAllObjects(QString::null);
}

QStringList SchemaResolver::getAllObjects(const QString& database)
{
    QStringList resList;
    QString dbName = getPrefixDb(database, db->getDialect());

    SqlResultsPtr results = db->exec(QString(
                "SELECT name, type FROM %1.sqlite_master;").arg(dbName));

    QString value;
    QString type;
    foreach (SqlResultsRowPtr row, results->getAll())
    {
        value = row->value("name").toString();
        type = row->value("type").toString();
        if (!isFilteredOut(value, type))
            resList << value;
    }

    return resList;
}

QString SchemaResolver::getUniqueName(const QString& database, const QString& namePrefix)
{
    QStringList allObjects = getAllObjects(database);
    QString baseName = namePrefix;
    QString name = baseName;
    for (int i = 0; allObjects.contains(name); i++)
        name = baseName + QString::number(i);

    return name;
}

QString SchemaResolver::getUniqueName(const QString& namePrefix)
{
    return getUniqueName("main", namePrefix);
}

QStringList SchemaResolver::getFkReferencingTables(const QString& table)
{
    return getFkReferencingTables("main", table);
}

QStringList SchemaResolver::getFkReferencingTables(const QString& database, const QString& table)
{
    Dialect dialect = db->getDialect();
    if (dialect == Dialect::Sqlite2)
        return QStringList();

    // Get all tables
    QStringList tables = getTables(database);

    // Exclude queried table from the list
    int idx = indexOf(tables, table, Qt::CaseInsensitive);
    if (idx > -1)
        tables.removeAt(idx);

    // For each table ask for its foreign keys and check if they reference queried table.
    QStringList theList;
    SqlResultsPtr results;
    SqlResultsRowPtr row;
    foreach (const QString& fkTable, tables)
    {
        results = db->exec(QString("PRAGMA %1.foreign_key_list(%2)").arg(wrapObjIfNeeded(database, dialect), wrapObjIfNeeded(fkTable, dialect)));

        if (results->isError())
        {
            qWarning() << "Could not get FK list for the table by the schema resolver:" << results->getErrorText()
                       << "The table will be excluded from results.";
            continue;
        }

        // We need just one fk to reference queried table and that's enough.
        foreach (row, results->getAll())
        {
            if (row->value("table").toString().compare(table, Qt::CaseInsensitive) == 0 && !theList.contains(fkTable, Qt::CaseInsensitive))
            {
                theList << fkTable;
                continue;
            }
        }
    }

    return theList;
}

QStringList SchemaResolver::getIndexesForTable(const QString& database, const QString& table)
{
    QStringList names;
    foreach (SqliteCreateIndexPtr idx, getParsedIndexesForTable(database, table))
        names << idx->index;

    return names;
}

QStringList SchemaResolver::getIndexesForTable(const QString& table)
{
    return getIndexesForTable("main", table);
}

QStringList SchemaResolver::getIndexesForView(const QString& database, const QString& table)
{
    return getIndexesForTable(database, table);
}

QStringList SchemaResolver::getIndexesForView(const QString& table)
{
    return getIndexesForTable(table);
}

QStringList SchemaResolver::getTriggersForTable(const QString& database, const QString& table)
{
    QStringList names;
    foreach (SqliteCreateTriggerPtr trig, getParsedTriggersForTable(database, table))
        names << trig->trigger;

    return names;
}

QStringList SchemaResolver::getTriggersForTable(const QString& table)
{
    return getTriggersForTable("main", table);
}

QStringList SchemaResolver::getViewsForTable(const QString& database, const QString& table)
{
    QStringList names;
    foreach (SqliteCreateViewPtr view, getParsedViewsForTable(database, table))
        names << view->view;

    return names;
}

QStringList SchemaResolver::getViewsForTable(const QString& table)
{
    return getViewsForTable("main", table);
}

QHash<QString, SchemaResolver::ObjectDetails> SchemaResolver::getAllObjectDetails()
{
    return getAllObjectDetails("main");
}

QHash<QString, SchemaResolver::ObjectDetails> SchemaResolver::getAllObjectDetails(const QString& database)
{
    QHash<QString, ObjectDetails> details;
    ObjectDetails detail;
    QString type;

    SqlResultsPtr results = db->exec(QString("SELECT name, type, sql FROM %1.sqlite_master").arg(getPrefixDb(database, db->getDialect())));
    if (results->isError())
    {
        qCritical() << "Error while getting all object details in SchemaResolver:" << results->getErrorCode();
        return details;
    }

    SqlResultsRowPtr row;
    while (results->hasNext())
    {
        row = results->next();
        type = row->value("type").toString();
        if (type == "table")
            detail.type = ObjectDetails::TABLE;
        else if (type == "index")
            detail.type = ObjectDetails::INDEX;
        else if (type == "trigger")
            detail.type = ObjectDetails::TRIGGER;
        else if (type == "view")
            detail.type = ObjectDetails::VIEW;
        else
            qCritical() << "Unhlandled db object type:" << type;

        detail.ddl = row->value("sql").toString();
        details[row->value("name").toString()] = detail;
    }
    return details;
}

QList<SqliteCreateIndexPtr> SchemaResolver::getParsedIndexesForTable(const QString& database, const QString& table)
{
    QList<SqliteCreateIndexPtr> createIndexList;

    QStringList indexes = getIndexes(database);
    SqliteQueryPtr query;
    SqliteCreateIndexPtr createIndex;
    foreach (const QString& index, indexes)
    {
        query = getParsedObject(database, index);
        if (!query)
            continue;

        createIndex = query.dynamicCast<SqliteCreateIndex>();
        if (!createIndex)
        {
            qWarning() << "Parsed DDL was not a CREATE INDEX statement, while queried for indexes.";
            continue;
        }

        if (createIndex->table.compare(table, Qt::CaseInsensitive) == 0)
            createIndexList << createIndex;
    }
    return createIndexList;
}

QList<SqliteCreateIndexPtr> SchemaResolver::getParsedIndexesForTable(const QString& table)
{
    return getParsedIndexesForTable("main", table);
}

QList<SqliteCreateTriggerPtr> SchemaResolver::getParsedTriggersForTable(const QString& database, const QString& table,
                                                                        bool includeContentReferences)
{
    return getParsedTriggersForTableOrView(database, table, includeContentReferences, true);
}

QList<SqliteCreateTriggerPtr> SchemaResolver::getParsedTriggersForTable(const QString& table, bool includeContentReferences)
{
    return getParsedTriggersForTable("main", table, includeContentReferences);
}

QList<SqliteCreateViewPtr> SchemaResolver::getParsedViewsForTable(const QString& database, const QString& table)
{
    QList<SqliteCreateViewPtr> createViewList;

    QStringList views = getViews(database);
    SqliteQueryPtr query;
    SqliteCreateViewPtr createView;
    foreach (const QString& view, views)
    {
        query = getParsedObject(database, view);
        if (!query)
            continue;

        createView = query.dynamicCast<SqliteCreateView>();
        if (!createView)
        {
            qWarning() << "Parsed DDL was not a CREATE VIEW statement, while queried for views.";
            continue;
        }

        if (indexOf(createView->getContextTables(), table, Qt::CaseInsensitive) > -1)
            createViewList << createView;
    }
    return createViewList;
}

QList<SqliteCreateViewPtr> SchemaResolver::getParsedViewsForTable(const QString& table)
{
    return getParsedViewsForTable("main", table);
}

QList<SqliteCreateTriggerPtr> SchemaResolver::getParsedTriggersForTableOrView(const QString& database, const QString& tableOrView, bool includeContentReferences, bool table)
{
    QList<SqliteCreateTriggerPtr> createTriggerList;

    QStringList triggers = getTriggers(database);
    SqliteQueryPtr query;
    SqliteCreateTriggerPtr createTrigger;
    foreach (const QString& trig, triggers)
    {
        query = getParsedObject(database, trig);
        if (!query)
            continue;

        createTrigger = query.dynamicCast<SqliteCreateTrigger>();
        if (!createTrigger)
        {
            qWarning() << "Parsed DDL was not a CREATE TRIGGER statement, while queried for triggers.";
            continue;
        }

        // The condition below checks:
        // 1. if this is a call for table triggers and event time is INSTEAD_OF - skip this iteration
        // 2. if this is a call for view triggers and event time is _not_ INSTEAD_OF - skip this iteration
        // In other words, it's a logical XOR for "table" flag and "eventTime == INSTEAD_OF" condition.
        if (table == (createTrigger->eventTime == SqliteCreateTrigger::Time::INSTEAD_OF))
            continue;

        if (createTrigger->table.compare(tableOrView, Qt::CaseInsensitive) == 0)
            createTriggerList << createTrigger;
        else if (includeContentReferences && indexOf(createTrigger->getContextTables(), tableOrView, Qt::CaseInsensitive) > -1)
            createTriggerList << createTrigger;

    }
    return createTriggerList;
}

void SchemaResolver::filterSystemIndexes(QStringList& indexes)
{
    Dialect dialect = db->getDialect();
    QMutableListIterator<QString> it(indexes);
    while (it.hasNext())
    {
        if (isSystemIndex(it.next(), dialect))
            it.remove();
    }
}

QList<SqliteCreateTriggerPtr> SchemaResolver::getParsedTriggersForView(const QString& database, const QString& view, bool includeContentReferences)
{
    return getParsedTriggersForTableOrView(database, view, includeContentReferences, false);
}

QList<SqliteCreateTriggerPtr> SchemaResolver::getParsedTriggersForView(const QString& view, bool includeContentReferences)
{
    return getParsedTriggersForView("main", view, includeContentReferences);
}

bool SchemaResolver::isWithoutRowIdTable(const QString& table)
{
    return isWithoutRowIdTable("main", table);
}

bool SchemaResolver::isWithoutRowIdTable(const QString& database, const QString& table)
{
    SqliteQueryPtr query = getParsedObject(database, table);
    if (!query)
        return false;

    SqliteCreateTablePtr createTable = query.dynamicCast<SqliteCreateTable>();
    if (!createTable)
        return false;

    return !createTable->withOutRowId.isNull();
}

bool SchemaResolver::isVirtualTable(const QString& database, const QString& table)
{
    SqliteQueryPtr query = getParsedObject(database, table);
    if (!query)
        return false;

    SqliteCreateVirtualTablePtr createVirtualTable = query.dynamicCast<SqliteCreateVirtualTable>();
    return !createVirtualTable.isNull();
}

bool SchemaResolver::isVirtualTable(const QString& table)
{
    return isVirtualTable("main", table);
}

QStringList SchemaResolver::getWithoutRowIdTableColumns(const QString& table)
{
    return getWithoutRowIdTableColumns("main", table);
}

QStringList SchemaResolver::getWithoutRowIdTableColumns(const QString& database, const QString& table)
{
    QStringList columns;

    SqliteQueryPtr query = getParsedObject(database, table);
    if (!query)
        return columns;

    SqliteCreateTablePtr createTable = query.dynamicCast<SqliteCreateTable>();
    if (!createTable)
        return columns;

    if (createTable->withOutRowId.isNull())
        return columns; // it's not WITHOUT ROWID table

    return createTable->getPrimaryKeyColumns();
}

QString SchemaResolver::getSqliteMasterDdl(bool temp)
{
    if (temp)
        return sqliteTempMasterDdl;

    return sqliteMasterDdl;
}

QStringList SchemaResolver::getCollations()
{
    QStringList list;
    if (db->getDialect() != Dialect::Sqlite3)
        return list;

    SqlResultsPtr results = db->exec("PRAGMA collation_list");
    if (results->isError())
    {
        qWarning() << "Could not read collation list from the database:" << results->getErrorText();
        return list;
    }

    SqlResultsRowPtr row;
    while (results->hasNext())
    {
        row = results->next();
        list << row->value("name").toString();
    }

    return list;
}

bool SchemaResolver::getIgnoreSystemObjects() const
{
    return ignoreSystemObjects;
}

void SchemaResolver::setIgnoreSystemObjects(bool value)
{
    ignoreSystemObjects = value;
}

