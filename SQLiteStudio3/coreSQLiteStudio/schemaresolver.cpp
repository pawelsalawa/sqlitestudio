#include "schemaresolver.h"
#include "db/db.h"
#include "db/sqlresultsrow.h"
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

ExpiringCache<SchemaResolver::ObjectCacheKey,QVariant> SchemaResolver::cache;
ExpiringCache<QString, QString> SchemaResolver::autoIndexDdlCache;

SchemaResolver::SchemaResolver(Db *db)
    : db(db)
{
    parser = new Parser();
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
    static_qstring(idxForTableTpl, "SELECT name FROM %1.pragma_index_list(%2)");

    QStringList tables = getTables(database);
    QStringList queryParts;
    for (const QString& table : tables)
        queryParts << idxForTableTpl.arg(wrapObjName(database), wrapString(table));

    QString query = queryParts.join(" UNION ");
    SqlQueryPtr results = db->exec(query, dbFlags);

    QStringList indexes;
    QString value;
    for (SqlResultsRowPtr row : results->getAll())
    {
        value = row->value(0).toString();
        if (isFilteredOut(value, "index"))
            continue;

        indexes << value;
    }

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

StrHash<QStringList> SchemaResolver::getGroupedIndexes(const QString &database)
{
    StrHash<QString> indexesWithTables = getIndexesWithTables(database);

    StrHash<QStringList> groupedIndexes;
    auto it = indexesWithTables.iterator();
    while (it.hasNext())
    {
        auto entry = it.next();
        groupedIndexes[entry.value()] << entry.key();
    }

    return groupedIndexes;
}

StrHash<QStringList> SchemaResolver::getGroupedTriggers(const QString &database)
{
    QStringList allTriggers = getTriggers(database);
    return getGroupedObjects(database, allTriggers, SqliteQueryType::CreateTrigger);
}

StrHash<QStringList> SchemaResolver::getGroupedObjects(const QString &database, const QStringList &inputList, SqliteQueryType type)
{
    QString strType = sqliteQueryTypeToString(type);
    ObjectType objectType = objectTypeFromQueryType(type);
    StrHash<QStringList> groupedObjects;

    SqliteQueryPtr parsedQuery;
    SqliteTableRelatedDdlPtr tableRelatedDdl;

    for (const QString& object : inputList)
    {
        parsedQuery = getParsedObject(database, object, objectType);
        if (!parsedQuery)
        {
            qWarning() << "Could not get parsed object for " << strType << ":" << object;
            continue;
        }

        tableRelatedDdl  = parsedQuery.dynamicCast<SqliteTableRelatedDdl>();
        if (!tableRelatedDdl)
        {
            qWarning() << "Parsed object is not of expected type. Expected" << strType
                       << ", but got" << sqliteQueryTypeToString(parsedQuery->queryType)
                       << "; Object db and name:" << database << object;
            continue;
        }

        groupedObjects[tableRelatedDdl->getTargetTable()] << object;
    }

    return groupedObjects;
}

bool SchemaResolver::isFilteredOut(const QString& value, const QString& type)
{
    if (ignoreSystemObjects)
    {
        if (type == "table" && isSystemTable(value))
            return true;

        if (type == "index" && isSystemIndex(value))
            return true;
    }

    return false;
}

QSet<QString> SchemaResolver::getDatabases()
{
    return db->getAllAttaches();
}

QStringList SchemaResolver::getTableColumns(const QString& table, bool onlyReal)
{
    return getTableColumns("main", table, onlyReal);
}

QStringList SchemaResolver::getTableColumns(const QString &database, const QString &table, bool onlyReal)
{
    QStringList columns; // result

    SqliteQueryPtr query = getParsedObject(database, table, TABLE);
    if (!query)
        return columns;

    SqliteCreateTablePtr createTable = query.dynamicCast<SqliteCreateTable>();
    SqliteCreateVirtualTablePtr createVirtualTable = query.dynamicCast<SqliteCreateVirtualTable>();
    if (!createTable && !createVirtualTable)
    {
        qDebug() << "Parsed DDL is neither a CREATE TABLE or CREATE VIRTUAL TABLE statement. It's: "
                 << sqliteQueryTypeToString(query->queryType) << "when trying to parse DDL of" << database << table;

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
    for (SqliteCreateTable::Column* column : createTable->columns)
    {
        if (onlyReal && column->hasConstraint(SqliteCreateTable::Column::Constraint::GENERATED))
            continue;

        columns << column->name;
    }

    return columns;
}

StrHash<DataType> SchemaResolver::getTableColumnDataTypesByName(const QString &table)
{
    return getTableColumnDataTypesByName("main", table);
}

StrHash<DataType> SchemaResolver::getTableColumnDataTypesByName(const QString &database, const QString &table)
{
    StrHash<DataType> dataTypes;
    SqliteCreateTablePtr createTable = getParsedObject(database, table, TABLE).dynamicCast<SqliteCreateTable>();
    if (!createTable)
    {
        return dataTypes;
    }

    for (SqliteCreateTable::Column* col : createTable->columns)
    {
        if (!col->type)
        {
            dataTypes[col->name] = DataType();
            continue;
        }

        dataTypes[col->name] = col->type->toDataType();
    }
    return dataTypes;
}

QList<DataType> SchemaResolver::getTableColumnDataTypes(const QString& table, int expectedNumberOfTypes)
{
    return getTableColumnDataTypes("main", table, expectedNumberOfTypes);
}

QList<DataType> SchemaResolver::getTableColumnDataTypes(const QString& database, const QString& table, int expectedNumberOfTypes)
{
    QList<DataType> dataTypes;
    SqliteCreateTablePtr createTable = getParsedObject(database, table, TABLE).dynamicCast<SqliteCreateTable>();
    if (!createTable)
    {
        for (int i = 0; i < expectedNumberOfTypes; i++)
            dataTypes << DataType();

        return dataTypes;
    }

    for (SqliteCreateTable::Column* col : createTable->columns)
    {
        if (!col->type)
        {
            dataTypes << DataType();
            continue;
        }

        dataTypes << col->type->toDataType();
    }

    for (int i = dataTypes.size(); i < expectedNumberOfTypes; i++)
        dataTypes << DataType();

    return dataTypes;
}

StrHash<QStringList> SchemaResolver::getAllTableColumns(const QString &database)
{
    StrHash< QStringList> tableColumns;
    for (QString table : getTables(database))
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
    for (const SelectResolver::Column& col : resolvedColumns)
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
    SqliteQueryPtr query = getParsedObject(database, view, VIEW);
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
    // The "table" name here used to be stripped with stripObjName(), but actually [abc] is a proper table name,
    // that should not be stripped. Any names passed to SchemaResolver cannot be wrapped.

    QString dbName = getPrefixDb(database);

    // Create temp table to see columns.
    QString newTable = db->getUniqueNewObjectName(table);
    QString origTable = wrapObjIfNeeded(table);
    SqlQueryPtr tempTableRes = db->exec(QString("CREATE TEMP TABLE %1 AS SELECT * FROM %2.%3 LIMIT 0;").arg(newTable, dbName, origTable), dbFlags);
    if (tempTableRes->isError())
        qWarning() << "Could not create temp table to identify virtual table columns of virtual table " << origTable << ". Error details:" << tempTableRes->getErrorText();

    // Get parsed DDL of the temp table.
    SqliteQueryPtr query = getParsedObject("temp", newTable, TABLE);
    if (!query)
        return SqliteCreateTablePtr();

    SqliteCreateTablePtr createTable = query.dynamicCast<SqliteCreateTable>();

    // Getting rid of the temp table.
    db->exec(QString("DROP TABLE %1;").arg(newTable), dbFlags);

    // Returning results. Might be null.
    return createTable;
}

QString SchemaResolver::getObjectDdl(const QString& name, ObjectType type)
{
    return getObjectDdl("main", name, type);
}

QString SchemaResolver::getObjectDdl(const QString &database, const QString &name, ObjectType type)
{
    // The "name" here used to be stripped with stripObjName(), but actually [abc] is a proper object name,
    // that should not be stripped. Any names passed to SchemaResolver cannot be wrapped.

    if (name.isNull())
        return QString();

    // Prepare db prefix.
    QString dbName = getPrefixDb(database);

    // In case of sqlite_master or sqlite_temp_master we have static definitions
    QString lowerName = name.toLower();
    if (lowerName == "sqlite_master")
        return getSqliteMasterDdl(false);
    else if (lowerName == "sqlite_temp_master")
        return getSqliteMasterDdl(true);
    else if (lowerName.startsWith("sqlite_autoindex_"))
        return getSqliteAutoIndexDdl(dbName, name);

    // Standalone or temp table?
    QString targetTable = "sqlite_master";
    if (database.toLower() == "temp")
        targetTable = "sqlite_temp_master";

    // Cache
    QString typeStr = objectTypeToString(type);
    bool useCache = usesCache();
    ObjectCacheKey key(ObjectCacheKey::OBJECT_DDL, db, dbName, lowerName, typeStr);
    if (useCache && cache.contains(key))
        return cache.object(key, true)->toString();

    // Get the DDL
    QString resStr = getObjectDdlWithSimpleName(dbName, lowerName, targetTable, type);
    if (resStr.isNull())
        resStr = getObjectDdlWithDifficultName(dbName, lowerName, targetTable, type);

    // If the DDL doesn't have semicolon at the end (usually the case), add it.
    if (!resStr.trimmed().endsWith(";"))
        resStr += ";";

    if (useCache)
        cache.insert(key, new QVariant(resStr));

    // Return the DDL
    return resStr;
}

QString SchemaResolver::getObjectDdlWithDifficultName(const QString &dbName, const QString &lowerName, QString targetTable, SchemaResolver::ObjectType type)
{
    //
    // Slower, but works with Russian names, etc, because "string lower" is done only at Qt level, not at SQLite level.
    //
    QString typeStr = objectTypeToString(type);
    SqlQueryPtr queryResults;
    if (type != ANY)
    {
        queryResults = db->exec(QString(
                    "SELECT name, sql FROM %1.%4 WHERE type = '%3';").arg(dbName, typeStr, targetTable),
                    dbFlags
                );

    }
    else
    {
        queryResults = db->exec(QString(
                    "SELECT name, sql FROM %1.%3;").arg(dbName, targetTable),
                    dbFlags
                );
    }

    // Validate query results
    if (queryResults->isError())
    {
        qDebug() << "Could not get object's DDL:" << dbName << "." << lowerName << ", details:" << queryResults->getErrorText();
        return QString();
    }

    // The DDL string
    SqlResultsRowPtr row;
    while (queryResults->hasNext())
    {
        row = queryResults->next();
        if (row->value("name").toString().toLower() != lowerName)
            continue;

        return row->value("sql").toString();
    }
    return QString();
}

QString SchemaResolver::getObjectDdlWithSimpleName(const QString &dbName, const QString &lowerName, QString targetTable, SchemaResolver::ObjectType type)
{
    QString typeStr = objectTypeToString(type);
    QVariant results;
    SqlQueryPtr queryResults;
    if (type != ANY)
    {
        queryResults = db->exec(QString(
                    "SELECT sql FROM %1.%4 WHERE lower(name) = '%2' AND type = '%3';").arg(dbName, escapeString(lowerName), typeStr, targetTable),
                    dbFlags
                );

    }
    else
    {
        queryResults = db->exec(QString(
                    "SELECT sql FROM %1.%3 WHERE lower(name) = '%2';").arg(dbName, escapeString(lowerName), targetTable),
                    dbFlags
                );
    }

    // Validate query results
    if (queryResults->isError())
    {
        qDebug() << "Could not get object's DDL:" << dbName << "." << lowerName << ", details:" << queryResults->getErrorText();
        return QString();
    }

    // The DDL string
    results = queryResults->getSingleCell();
    return results.toString();
}

StrHash<QString> SchemaResolver::getIndexesWithTables(const QString& database)
{
    static_qstring(idxForTableTpl, "SELECT %2 as tbl_name, name FROM %1.pragma_index_list(%2)");

    QStringList tables = getTables(database);
    QString dbName = getPrefixDb(database);
    QStringList queryParts;
    for (const QString& table : tables)
        queryParts << idxForTableTpl.arg(wrapObjName(dbName), wrapString(table));

    QString query = queryParts.join(" UNION ");
    SqlQueryPtr results = db->exec(query, dbFlags);

    StrHash<QString> indexes;
    QString tabName;
    QString idxName;
    for (SqlResultsRowPtr row : results->getAll())
    {
        tabName = row->value("tbl_name").toString();
        idxName = row->value("name").toString();
        if (isFilteredOut(idxName, "index"))
            continue;

        indexes[idxName] = tabName;
    }

    return indexes;
}

QStringList SchemaResolver::getColumnsFromDdlUsingPragma(const QString& ddl)
{
    Parser parser;
    if (!parser.parse(ddl) || parser.getQueries().isEmpty())
    {
        qWarning() << "Could not parse DDL for determinating columns using PRAGMA. The DDL was:\n" << ddl;
        return QStringList();
    }

    SqliteQueryPtr query = parser.getQueries().first();
    if (query->queryType == SqliteQueryType::CreateTable)
        return getColumnsUsingPragma(query.dynamicCast<SqliteCreateTable>().data());

    if (query->queryType == SqliteQueryType::CreateView)
        return getColumnsUsingPragma(query.dynamicCast<SqliteCreateView>().data());

    qWarning() << "Tried to get columns of DDL using pragma for statement other than table or view:" << sqliteQueryTypeToString(query->queryType) << "for DDL:\n" << ddl;
    return QStringList();
}

QStringList SchemaResolver::getColumnsUsingPragma(const QString& tableOrView)
{
    static_qstring(query, "PRAGMA table_info(%1)");
    SqlQueryPtr results = db->exec(query.arg(wrapObjIfNeeded(tableOrView)));
    if (results->isError())
    {
        qWarning() << "Could not get column list using PRAGMA for table or view:" << tableOrView << ", error was:" << results->getErrorText();
        return QStringList();
    }

    QStringList cols;
    for (const SqlResultsRowPtr& row : results->getAll())
        cols << row->value("name").toString();

    return cols;
}

QStringList SchemaResolver::getColumnsUsingPragma(SqliteCreateTable* createTable)
{
    QString name = getUniqueName();
    SqliteCreateTable* stmt = dynamic_cast<SqliteCreateTable*>(createTable->clone());
    stmt->tempKw = true;
    stmt->table = name;
    stmt->database = QString();
    stmt->rebuildTokens();
    QString ddl = stmt->tokens.detokenize();
    delete stmt;

    SqlQueryPtr result = db->exec(ddl);
    if (result->isError())
    {
        qWarning() << "Could not create table for finding its columns using PRAGMA. Error was:" << result->getErrorText();
        return QStringList();
    }

    QStringList columns = getColumnsUsingPragma(name);

    static_qstring(dropSql, "DROP TABLE %1");
    db->exec(dropSql.arg(wrapObjIfNeeded(name)));

    return columns;
}

QStringList SchemaResolver::getColumnsUsingPragma(SqliteCreateView* createView)
{
    QString name = getUniqueName();
    SqliteCreateView* stmt = dynamic_cast<SqliteCreateView*>(createView->clone());
    stmt->tempKw = true;
    stmt->view = name;
    stmt->database = QString();
    stmt->rebuildTokens();
    QString ddl = stmt->tokens.detokenize();
    delete stmt;

    SqlQueryPtr result = db->exec(ddl);
    if (result->isError())
    {
        qWarning() << "Could not create view for finding its columns using PRAGMA. Error was:" << result->getErrorText();
        return QStringList();
    }

    QStringList columns = getColumnsUsingPragma(name);

    static_qstring(dropSql, "DROP VIEW %1");
    db->exec(dropSql.arg(wrapObjIfNeeded(name)));

    return columns;
}

SqliteQueryPtr SchemaResolver::getParsedObject(const QString &name, ObjectType type)
{
    return getParsedObject("main", name, type);
}

SqliteQueryPtr SchemaResolver::getParsedObject(const QString &database, const QString &name, ObjectType type)
{
    // Get DDL
    QString ddl = getObjectDdl(database, name, type);
    if (ddl.isNull())
        return SqliteQueryPtr();

    // Parse DDL
    return getParsedDdl(ddl);
}

StrHash< SqliteQueryPtr> SchemaResolver::getAllParsedObjects()
{
    return getAllParsedObjects("main");
}

StrHash< SqliteQueryPtr> SchemaResolver::getAllParsedObjects(const QString& database)
{
    return getAllParsedObjectsForType<SqliteQuery>(database, QString());
}

StrHash< SqliteCreateTablePtr> SchemaResolver::getAllParsedTables()
{
    return getAllParsedTables("main");
}

StrHash< SqliteCreateTablePtr> SchemaResolver::getAllParsedTables(const QString& database)
{
    return getAllParsedObjectsForType<SqliteCreateTable>(database, "table");
}

StrHash< SqliteCreateIndexPtr> SchemaResolver::getAllParsedIndexes()
{
    return getAllParsedIndexes("main");
}

StrHash< SqliteCreateIndexPtr> SchemaResolver::getAllParsedIndexes(const QString& database)
{
    return getAllParsedObjectsForType<SqliteCreateIndex>(database, "index");
}

StrHash< SqliteCreateTriggerPtr> SchemaResolver::getAllParsedTriggers()
{
    return getAllParsedTriggers("main");
}

StrHash< SqliteCreateTriggerPtr> SchemaResolver::getAllParsedTriggers(const QString& database)
{
    return getAllParsedObjectsForType<SqliteCreateTrigger>(database, "trigger");
}

StrHash< SqliteCreateViewPtr> SchemaResolver::getAllParsedViews()
{
    return getAllParsedViews("main");
}

StrHash< SqliteCreateViewPtr> SchemaResolver::getAllParsedViews(const QString& database)
{
    return getAllParsedObjectsForType<SqliteCreateView>(database, "view");
}

QString SchemaResolver::getSqliteAutoIndexDdl(const QString& database, const QString& index)
{
    // First, let's try to use cached value
    static_qstring(cacheKeyTpl, "%1.%2");
    QString cacheKey = cacheKeyTpl.arg(database, index).toLower();
    QString* cachedDdlPtr = autoIndexDdlCache[cacheKey];
    if (cachedDdlPtr)
        return *(cachedDdlPtr);

    // Not in cache. We need to find out indexed table.
    // Let's try to find it in sqlite_master.
    // If it's there, we will at least know it's referenced table.
    static_qstring(masterQuery, "SELECT tbl_name FROM %1.sqlite_master WHERE type = 'index' AND name = ?");

    QString table;
    QString dbName = getPrefixDb(database);
    QVariant masterRes = db->exec(masterQuery.arg(dbName), {index}, dbFlags)->getSingleCell();
    if (masterRes.isNull())
    {
        // Not lucky. We need to find out the table.
        StrHash<QString> indexesWithTables = getIndexesWithTables(database);
        auto it = indexesWithTables.iterator();
        while (it.hasNext())
        {
            auto entry = it.next();
            if (entry.key().toLower() == index.toLower())
            {
                table = entry.value();
                break;
            }
        }
    }
    else
        table = masterRes.toString();

    if (table.isNull())
    {
        qCritical() << "Could not determin table associated with index" << database << "." << index;
        return QString();
    }

    // Check the unique flag of the index
    static_qstring(idxUniqueQueryTpl, "SELECT [unique] FROM %1.pragma_index_list(%2) WHERE name = ?");
    SqlQueryPtr uniqRes = db->exec(idxUniqueQueryTpl.arg(dbName, wrapString(table)), {index}, dbFlags);
    bool unique = uniqRes->getSingleCell().toInt() > 0;

    // Now let's find out columns
    static_qstring(idxColQueryTpl, "SELECT name, coll, desc FROM %1.pragma_index_xinfo(%2) WHERE key = 1");
    static_qstring(idxColTpl, "%1 COLLATE %2");

    QStringList columns;
    SqlQueryPtr colRes = db->exec(idxColQueryTpl.arg(dbName, wrapString(index)), dbFlags);
    while (colRes->hasNext())
    {
        SqlResultsRowPtr row = colRes->next();
        QString column = idxColTpl.arg(wrapObjIfNeeded(row->value("name").toString()), row->value("coll").toString());
        if (row->value("desc").toInt() > 0)
            column += " DESC";

        columns << column;
    }

    // Finally, let's build it up & cache
    static_qstring(ddlTpl, "CREATE %1INDEX %2 ON %3 (%4)");
    QString ddl = ddlTpl.arg(
                unique ? "UNIQUE " : "",
                wrapObjIfNeeded(index),
                wrapObjIfNeeded(table),
                columns.join(", ")
                );

    autoIndexDdlCache.insert(cacheKey, new QString(ddl));
    return ddl;
}

SqliteQueryPtr SchemaResolver::getParsedDdl(const QString& ddl)
{
    if (!parser->parse(ddl))
    {
        qDebug() << "Could not parse DDL for parsing object by SchemaResolver. Errors are:";
        for (ParserError* err : parser->getErrors())
            qDebug() << err->getMessage();

        qDebug() << "The DDL is:" << ddl;

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
    return getObjects(QString(), type);
}

QStringList SchemaResolver::getObjects(const QString &database, const QString &type)
{
    bool useCache = usesCache();
    ObjectCacheKey key(ObjectCacheKey::OBJECT_NAMES, db, database, type);
    if (useCache && cache.contains(key))
        return cache.object(key, true)->toStringList();

    QStringList resList;
    QString dbName = getPrefixDb(database);

    SqlQueryPtr results = db->exec(QString("SELECT name FROM %1.sqlite_master WHERE type = ?;").arg(dbName), {type}, dbFlags);

    QString value;
    for (SqlResultsRowPtr row : results->getAll())
    {
        value = row->value(0).toString();
        if (!isFilteredOut(value, type))
            resList << value;
    }

    if (useCache)
        cache.insert(key, new QVariant(resList));

    return resList;
}

QStringList SchemaResolver::getAllObjects()
{
    return getAllObjects(QString());
}

QStringList SchemaResolver::getAllObjects(const QString& database)
{
    bool useCache = usesCache();
    ObjectCacheKey key(ObjectCacheKey::OBJECT_NAMES, db, database);
    if (useCache && cache.contains(key))
        return cache.object(key, true)->toStringList();

    QStringList resList;
    QString dbName = getPrefixDb(database);

    SqlQueryPtr results = db->exec(QString("SELECT name, type FROM %1.sqlite_master;").arg(dbName), dbFlags);

    QString value;
    QString type;
    for (SqlResultsRowPtr row : results->getAll())
    {
        value = row->value("name").toString();
        type = row->value("type").toString();
        if (!isFilteredOut(value, type))
            resList << value;
    }

    if (useCache)
        cache.insert(key, new QVariant(resList));

    return resList;
}

QString SchemaResolver::getUniqueName(const QString& database, const QString& namePrefix, const QStringList& forbiddenNames)
{
    QStringList allObjects = getAllObjects(database);
    allObjects += forbiddenNames;
    QString baseName = namePrefix;
    QString name = baseName;
    for (int i = 0; allObjects.contains(name); i++)
        name = baseName + QString::number(i);

    return name;
}

QString SchemaResolver::getUniqueName(const QString& namePrefix, const QStringList& forbiddenNames)
{
    return getUniqueName("main", namePrefix, forbiddenNames);
}

QStringList SchemaResolver::getFkReferencingTables(const QString& table)
{
    return getFkReferencingTables("main", table);
}

QStringList SchemaResolver::getFkReferencingTables(const QString& database, const QString& table)
{
    static_qstring(fkQueryTpl, R"(
                WITH foreign_keys AS (
                   SELECT m.name AS table_name, lower(fk.[table]) AS foreign_table
                     FROM %1.sqlite_master AS m
                     JOIN %1.pragma_foreign_key_list(m.name) AS fk
                    WHERE m.type = 'table'
                )
                SELECT table_name
                  FROM foreign_keys
                 WHERE foreign_table = '%2';)");

    SqlQueryPtr results = db->exec(fkQueryTpl.arg(getPrefixDb(database), escapeString(table.toLower())), dbFlags);
    if (results->isError())
    {
        qCritical() << "Error while getting FK-referencing table list in SchemaResolver:" << results->getErrorCode();
        return QStringList();
    }

    QStringList resList;
    for (SqlResultsRowPtr row : results->getAll())
        resList << row->value(0).toString();

    return resList;
}

QStringList SchemaResolver::getFkReferencedTables(const QString& table)
{
    return getFkReferencedTables("main", table);
}

QStringList SchemaResolver::getFkReferencedTables(const QString& database, const QString& table)
{
    static_qstring(fkQueryTpl, "SELECT [table] FROM %1.pragma_foreign_key_list('%2');");

    SqlQueryPtr results = db->exec(fkQueryTpl.arg(getPrefixDb(database), escapeString(table)), dbFlags);
    if (results->isError())
    {
        qCritical() << "Error while getting FK-referenced table list in SchemaResolver:" << results->getErrorCode() << results->getErrorText();
        return QStringList();
    }

    QStringList resList;
    for (SqlResultsRowPtr row : results->getAll())
        resList << row->value(0).toString();

    return resList;
}

SchemaResolver::ObjectType SchemaResolver::objectTypeFromQueryType(const SqliteQueryType& queryType)
{
    switch (queryType)
    {
        case SqliteQueryType::CreateIndex:
            return INDEX;
        case SqliteQueryType::CreateTrigger:
            return TRIGGER;
        case SqliteQueryType::CreateView:
            return VIEW;
        case SqliteQueryType::CreateTable:
        case SqliteQueryType::CreateVirtualTable:
            return TABLE;
        case SqliteQueryType::Select:
        case SqliteQueryType::Pragma:
        case SqliteQueryType::UNDEFINED:
        case SqliteQueryType::EMPTY:
        case SqliteQueryType::AlterTable:
        case SqliteQueryType::Analyze:
        case SqliteQueryType::Attach:
        case SqliteQueryType::BeginTrans:
        case SqliteQueryType::CommitTrans:
        case SqliteQueryType::Copy:
        case SqliteQueryType::Delete:
        case SqliteQueryType::Detach:
        case SqliteQueryType::DropIndex:
        case SqliteQueryType::DropTable:
        case SqliteQueryType::DropTrigger:
        case SqliteQueryType::DropView:
        case SqliteQueryType::Insert:
        case SqliteQueryType::Reindex:
        case SqliteQueryType::Release:
        case SqliteQueryType::Rollback:
        case SqliteQueryType::Savepoint:
        case SqliteQueryType::Update:
        case SqliteQueryType::Vacuum:
            return ANY;
    }
    return ANY;
}

QStringList SchemaResolver::getIndexesForTable(const QString& database, const QString& table)
{
    static_qstring(idxForTableTpl, "SELECT name FROM %1.sqlite_master WHERE type = 'index' AND (tbl_name = '%2' OR lower(tbl_name) = lower('%2'));");

    QString query = idxForTableTpl.arg(wrapObjName(database), wrapObjIfNeeded(table));
    SqlQueryPtr results = db->exec(query, dbFlags);

    QStringList indexes;
    QString value;
    for (SqlResultsRowPtr row : results->getAll())
    {
        value = row->value(0).toString();
        if (isFilteredOut(value, "index"))
            continue;

        indexes << value;
    }

    return indexes;
}

QStringList SchemaResolver::getIndexesForTable(const QString& table)
{
    return getIndexesForTable("main", table);
}

QStringList SchemaResolver::getTriggersForTable(const QString& database, const QString& table)
{
    static_qstring(trigForTableTpl, "SELECT name FROM %1.sqlite_master WHERE type = 'trigger' AND (tbl_name = '%2' OR lower(tbl_name) = lower('%2'));");

    QString query = trigForTableTpl.arg(wrapObjName(database), escapeString(table));
    SqlQueryPtr results = db->exec(query, dbFlags);

    QStringList names;
    for (SqlResultsRowPtr row : results->getAll())
        names << row->value(0).toString();

    return names;
}

QStringList SchemaResolver::getTriggersForTable(const QString& table)
{
    return getTriggersForTable("main", table);
}

QStringList SchemaResolver::getTriggersForView(const QString& database, const QString& view)
{
    QStringList names;
    for (SqliteCreateTriggerPtr trig : getParsedTriggersForView(database, view))
        names << trig->trigger;

    return names;
}

QStringList SchemaResolver::getTriggersForView(const QString& view)
{
    return getTriggersForView("main", view);
}

QStringList SchemaResolver::getViewsForTable(const QString& database, const QString& table)
{
    QStringList names;
    for (SqliteCreateViewPtr view : getParsedViewsForTable(database, table))
        names << view->view;

    return names;
}

QStringList SchemaResolver::getViewsForTable(const QString& table)
{
    return getViewsForTable("main", table);
}

QStringList SchemaResolver::getIndexDdlsForTable(const QString& database, const QString& table)
{
    return getObjectDdlsReferencingTableOrView(database, table, INDEX);
}

QStringList SchemaResolver::getIndexDdlsForTable(const QString& table)
{
    return getIndexDdlsForTable("main", table);
}

QStringList SchemaResolver::getTriggerDdlsForTableOrView(const QString& database, const QString& table)
{
    return getObjectDdlsReferencingTableOrView(database, table, TRIGGER);
}

QStringList SchemaResolver::getTriggerDdlsForTableOrView(const QString& table)
{
    return getTriggerDdlsForTableOrView("main", table);
}

QList<SchemaResolver::TableListItem> SchemaResolver::getAllTableListItems()
{
    return getAllTableListItems("main");
}

QList<SchemaResolver::TableListItem> SchemaResolver::getAllTableListItems(const QString& database)
{
    QList<TableListItem> items;

    QList<QVariant> rows;
    bool useCache = usesCache();
    ObjectCacheKey key(ObjectCacheKey::TABLE_LIST_ITEM, db, database);
    if (useCache && cache.contains(key))
    {
        rows = cache.object(key, true)->toList();
    }
    else
    {
        //SqlQueryPtr results = db->exec(QString("PRAGMA %1.table_list").arg(getPrefixDb(database)), dbFlags); // not using for now to support SQLite versions < 3.37.0
        static_qstring(queryTpl, "SELECT name, (CASE WHEN type = 'view' THEN 'view' WHEN sql LIKE 'CREATE VIRTUAL%' THEN 'virtual' ELSE 'table' END) AS type FROM %1.sqlite_master WHERE type IN ('table', 'view')");
        SqlQueryPtr results = db->exec(queryTpl.arg(getPrefixDb(database)), dbFlags);
        if (results->isError())
        {
            qCritical() << "Error while getting all table list items in SchemaResolver:" << results->getErrorCode();
            return items;
        }

        for (const SqlResultsRowPtr& row : results->getAll())
            rows << row->valueMap();

        if (!ignoreSystemObjects)
        {
            static QHash<QString, QVariant> sqliteMasterRow {
                {"name", "sqlite_master"},
                {"type", "table"}
            };

            static QHash<QString, QVariant> sqliteTempMasterRow {
                {"name", "sqlite_temp_master"},
                {"type", "table"}
            };

            rows << QVariant(sqliteMasterRow) << QVariant(sqliteTempMasterRow);
        }

        if (useCache)
            cache.insert(key, new QVariant(rows));
    }

    QHash<QString, QVariant> row;
    for (const QVariant& rowVariant : rows)
    {
        row = rowVariant.toHash();
        QString value = row["name"].toString();
        QString type = row["type"].toString();
        if (isFilteredOut(value, type))
            continue;

        TableListItem item;
        item.type = stringToTableListItemType(type);
        if (item.type == TableListItem::UNKNOWN)
            qCritical() << "Unhlandled table item type:" << type;

        item.name = value;
        items << item;
    }

    return items;
}

StrHash<SchemaResolver::ObjectDetails> SchemaResolver::getAllObjectDetails()
{
    return getAllObjectDetails("main");
}

StrHash<SchemaResolver::ObjectDetails> SchemaResolver::getAllObjectDetails(const QString& database)
{
    StrHash<ObjectDetails> details;
    ObjectDetails detail;
    QString type;

    QList<QVariant> rows;
    bool useCache = usesCache();
    ObjectCacheKey key(ObjectCacheKey::OBJECT_DETAILS, db, ignoreSystemObjects, database);
    if (useCache && cache.contains(key))
    {
        rows = cache.object(key, true)->toList();
    }
    else
    {
        SqlQueryPtr results = db->exec(QString("SELECT name, type, sql FROM %1.sqlite_master").arg(getPrefixDb(database)), dbFlags);
        if (results->isError())
        {
            qCritical() << "Error while getting all object details in SchemaResolver:" << results->getErrorCode();
            return details;
        }

        for (const SqlResultsRowPtr& row : results->getAll())
        {
            if (isFilteredOut(row->value("name").toString(), row->value("type").toString()))
                continue;

            rows << row->valueMap();
        }

        if (useCache)
            cache.insert(key, new QVariant(rows));
    }

    QHash<QString, QVariant> row;
    for (const QVariant& rowVariant : rows)
    {
        row = rowVariant.toHash();
        type = row["type"].toString();
        detail.type = stringToObjectType(type);
        if (detail.type == ANY)
            qCritical() << "Unhlandled db object type:" << type;

        detail.ddl = row["sql"].toString();
        details[row["name"].toString()] = detail;
    }

    return details;
}

QList<SqliteCreateIndexPtr> SchemaResolver::getParsedIndexesForTable(const QString& database, const QString& table)
{
    static_qstring(idxForTableTpl, "SELECT sql, name FROM %1.sqlite_master WHERE type = 'index' AND lower(tbl_name) = lower(?);");

    QString query = idxForTableTpl.arg(getPrefixDb(database));
    SqlQueryPtr results = db->exec(query, {table}, dbFlags);

    QList<SqliteCreateIndexPtr> createIndexList;
    for (SqlResultsRowPtr row : results->getAll())
    {
        QString ddl = row->value(0).toString();
        QString name = row->value(1).toString();
        if (ddl.isEmpty() || isFilteredOut(name, "index"))
            continue;

        SqliteQueryPtr query = getParsedDdl(ddl);
        if (!query)
            continue;

        SqliteCreateIndexPtr createIndex = query.dynamicCast<SqliteCreateIndex>();
        if (!createIndex)
        {
            qWarning() << "Parsed DDL was not a CREATE INDEX statement, while queried for indexes. Queried db & table:"
                       << database << table << "Index name:" << name << "DDL:" << ddl;
            continue;
        }
        createIndexList << createIndex;
    }

    return createIndexList;
}

QList<SqliteCreateIndexPtr> SchemaResolver::getParsedIndexesForTable(const QString& table)
{
    return getParsedIndexesForTable("main", table);
}

QList<SqliteCreateTriggerPtr> SchemaResolver::getParsedTriggersForTable(const QString& database, const QString& table, bool includeContentReferences)
{
    return getParsedTriggersForTableOrView(database, table, includeContentReferences);
}

QList<SqliteCreateTriggerPtr> SchemaResolver::getParsedTriggersForTable(const QString& table, bool includeContentReferences)
{
    return getParsedTriggersForTable("main", table, includeContentReferences);
}

QList<SqliteCreateTriggerPtr> SchemaResolver::getParsedTriggersForView(const QString& database, const QString& view, bool includeContentReferences)
{
    return getParsedTriggersForTableOrView(database, view, includeContentReferences);
}

QList<SqliteCreateTriggerPtr> SchemaResolver::getParsedTriggersForView(const QString& view, bool includeContentReferences)
{
    return getParsedTriggersForView("main", view, includeContentReferences);
}

QList<SqliteCreateTriggerPtr> SchemaResolver::getParsedTriggersForTableOrView(const QString& database, const QString& tableOrView,
                                                                        bool includeContentReferences)
{
    static_qstring(trigForTableTpl, "SELECT sql, name FROM %1.sqlite_master WHERE type = 'trigger' AND lower(tbl_name) = lower('%2');");
    static_qstring(allTrigTpl, "SELECT sql FROM %1.sqlite_master WHERE type = 'trigger' AND lower(name) NOT IN (%2);");

    QString query = trigForTableTpl.arg(getPrefixDb(database), escapeString(tableOrView));
    SqlQueryPtr results = db->exec(query, dbFlags);

    QStringList alreadyProcessed;
    QList<SqliteCreateTriggerPtr> createTriggerList;
    for (SqlResultsRowPtr row : results->getAll())
    {
        alreadyProcessed << wrapString(escapeString(row->value(1).toString().toLower()));
        SqliteQueryPtr parsedDdl = getParsedDdl(row->value(0).toString());
        if (!parsedDdl)
            continue;

        SqliteCreateTriggerPtr createTrigger = parsedDdl.dynamicCast<SqliteCreateTrigger>();
        if (!createTrigger)
        {
            qWarning() << "Parsed DDL was not a CREATE TRIGGER statement, while queried for triggers.";
            continue;
        }
        createTriggerList << createTrigger;
    }

    if (includeContentReferences)
    {
        query = allTrigTpl.arg(wrapObjName(database), alreadyProcessed.join(", "));
        results = db->exec(query, dbFlags);

        for (SqlResultsRowPtr row : results->getAll())
        {
            SqliteQueryPtr parsedDdl = getParsedDdl(row->value(0).toString());
            if (!parsedDdl)
                continue;

            SqliteCreateTriggerPtr createTrigger = parsedDdl.dynamicCast<SqliteCreateTrigger>();
            if (!createTrigger)
            {
                qWarning() << "Parsed DDL was not a CREATE TRIGGER statement, while queried for triggers.";
                continue;
            }
            if (indexOf(createTrigger->getContextTables(), tableOrView, Qt::CaseInsensitive) > -1)
                createTriggerList << createTrigger;
        }
    }

    return createTriggerList;

//    QList<SqliteCreateTriggerPtr> createTriggerList;

//    QStringList triggers = getTriggersForTable(database);
//    SqliteQueryPtr query;
//    SqliteCreateTriggerPtr createTrigger;
//    for (const QString& trig : triggers)
//    {
//        query = getParsedObject(database, trig, TRIGGER);
//        if (!query)
//            continue;

//        createTrigger = query.dynamicCast<SqliteCreateTrigger>();
//        if (!createTrigger)
//        {
//            qWarning() << "Parsed DDL was not a CREATE TRIGGER statement, while queried for triggers." << createTrigger.data();
//            continue;
//        }

//        // The condition below checks:
//        // 1. if this is a call for table triggers and event time is INSTEAD_OF - skip this iteration
//        // 2. if this is a call for view triggers and event time is _not_ INSTEAD_OF - skip this iteration
//        // In other words, it's a logical XOR for "table" flag and "eventTime == INSTEAD_OF" condition.
//        if (table == (createTrigger->eventTime == SqliteCreateTrigger::Time::INSTEAD_OF))
//            continue;

//        if (createTrigger->table.compare(tableOrView, Qt::CaseInsensitive) == 0)
//            createTriggerList << createTrigger;
//        else if (includeContentReferences && indexOf(createTrigger->getContextTables(), tableOrView, Qt::CaseInsensitive) > -1)
//            createTriggerList << createTrigger;

//    }
//    return createTriggerList;
}

QString SchemaResolver::objectTypeToString(SchemaResolver::ObjectType type)
{
    switch (type)
    {
        case TABLE:
            return "table";
        case INDEX:
            return "index";
        case TRIGGER:
            return "trigger";
        case VIEW:
            return "view";
        case ANY:
            return QString();
    }
    return QString();
}

SchemaResolver::ObjectType SchemaResolver::stringToObjectType(const QString& type)
{
    if (type == "table")
        return SchemaResolver::TABLE;
    else if (type == "index")
        return SchemaResolver::INDEX;
    else if (type == "trigger")
        return SchemaResolver::TRIGGER;
    else if (type == "view")
        return SchemaResolver::VIEW;
    else
        return SchemaResolver::ANY;
}

SchemaResolver::TableListItem::Type SchemaResolver::stringToTableListItemType(const QString& type)
{
    if (type == "table")
        return SchemaResolver::TableListItem::TABLE;
    else if (type == "virtual")
        return SchemaResolver::TableListItem::VIRTUAL_TABLE;
    else if (type == "shadow")
        return SchemaResolver::TableListItem::SHADOW_TABLE;
    else if (type == "view")
        return SchemaResolver::TableListItem::VIEW;
    else
        return SchemaResolver::TableListItem::UNKNOWN;
}

void SchemaResolver::staticInit()
{
    cache.setExpireTime(3000);
}

bool SchemaResolver::usesCache()
{
    return db->getConnectionOptions().contains(USE_SCHEMA_CACHING) && db->getConnectionOptions()[USE_SCHEMA_CACHING].toBool();
}

QList<SqliteCreateViewPtr> SchemaResolver::getParsedViewsForTable(const QString& database, const QString& table)
{
    QList<SqliteCreateViewPtr> createViewList;

    QStringList views = getViews(database);
    SqliteQueryPtr query;
    SqliteCreateViewPtr createView;
    for (const QString& view : views)
    {
        query = getParsedObject(database, view, VIEW);
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

void SchemaResolver::filterSystemIndexes(QStringList& indexes)
{
    QMutableListIterator<QString> it(indexes);
    while (it.hasNext())
    {
        if (isSystemIndex(it.next()))
            it.remove();
    }
}

bool SchemaResolver::isWithoutRowIdTable(const QString& table)
{
    return isWithoutRowIdTable("main", table);
}

bool SchemaResolver::isWithoutRowIdTable(const QString& database, const QString& table)
{
    SqliteQueryPtr query = getParsedObject(database, table, TABLE);
    if (!query)
        return false;

    SqliteCreateTablePtr createTable = query.dynamicCast<SqliteCreateTable>();
    if (!createTable)
        return false;

    return createTable->withOutRowId;
}

bool SchemaResolver::isVirtualTable(const QString& database, const QString& table)
{
    QString ddl = getObjectDdl(database, table, TABLE);
    return ddl.simplified().toUpper().startsWith("CREATE VIRTUAL TABLE");
}

bool SchemaResolver::isVirtualTable(const QString& table)
{
    return isVirtualTable("main", table);
}

SqliteCreateTablePtr SchemaResolver::resolveVirtualTableAsRegularTable(const QString& table)
{
    return resolveVirtualTableAsRegularTable("maine", table);
}

SqliteCreateTablePtr SchemaResolver::resolveVirtualTableAsRegularTable(const QString& database, const QString& table)
{
    return virtualTableAsRegularTable(database, table);
}

QStringList SchemaResolver::getWithoutRowIdTableColumns(const QString& table)
{
    return getWithoutRowIdTableColumns("main", table);
}

QStringList SchemaResolver::getWithoutRowIdTableColumns(const QString& database, const QString& table)
{
    QStringList columns;

    SqliteQueryPtr query = getParsedObject(database, table, TABLE);
    if (!query)
        return columns;

    SqliteCreateTablePtr createTable = query.dynamicCast<SqliteCreateTable>();
    if (!createTable)
        return columns;

    if (!createTable->withOutRowId)
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
    SqlQueryPtr results = db->exec("PRAGMA collation_list", dbFlags);
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

bool SchemaResolver::getNoDbLocking() const
{
    return dbFlags.testFlag(Db::Flag::NO_LOCK);
}

void SchemaResolver::setNoDbLocking(bool value)
{
    if (value)
        dbFlags |= Db::Flag::NO_LOCK;
    else
        dbFlags ^= Db::Flag::NO_LOCK;
}

QString SchemaResolver::normalizeCaseObjectName(const QString& name)
{
    static_qstring(sql, "SELECT name FROM main.sqlite_master WHERE lower(name) = lower(?);");
    return normalizeCaseObjectNameByQuery(sql, name);
}

QString SchemaResolver::normalizeCaseObjectName(const QString& database, const QString& name)
{
    static_qstring(sql, "SELECT name FROM %1.sqlite_master WHERE lower(name) = lower(?);");
    QString query = sql.arg(wrapObjIfNeeded(database));
    return normalizeCaseObjectNameByQuery(query, name);
}

QString SchemaResolver::normalizeCaseObjectNameByQuery(const QString& query, const QString& name)
{
    SqlQueryPtr results = db->exec(query, {name});
    if (results->isError())
    {
        qCritical() << "Could not get object name normalized case. Object name:" << name << ", error:"
                    << results->getErrorText();
        return name;
    }

    return results->getSingleCell().toString();
}

QStringList SchemaResolver::getObjectDdlsReferencingTableOrView(const QString& database, const QString& table, ObjectType type)
{
    static_qstring(trigForTableTpl, "SELECT sql FROM %1.sqlite_master WHERE type = '%3' AND (tbl_name = '%2' OR lower(tbl_name) = lower('%2'));"); // non-lower variant for cyrlic alphabet

    QString query = trigForTableTpl.arg(wrapObjName(database), escapeString(table), objectTypeToString(type));
    SqlQueryPtr results = db->exec(query, dbFlags);

    QStringList ddls;
    for (SqlResultsRowPtr row : results->getAll())
    {
        QString ddl = row->value(0).toString();
        if (!ddl.trimmed().endsWith(";"))
            ddl += ";";

        ddls << ddl;
    }

    return ddls;

}

SchemaResolver::ObjectCacheKey::ObjectCacheKey(Type type, Db* db, const QString& value1, const QString& value2, const QString& value3) :
  ObjectCacheKey(type, db, false, value1, value2, value3)
{
}

SchemaResolver::ObjectCacheKey::ObjectCacheKey(Type type, Db* db, bool skipSystemObj, const QString& value1, const QString& value2, const QString& value3) :
    type(type), db(db), skipSystemObj(skipSystemObj), value1(value1), value2(value2), value3(value3)
{
}

TYPE_OF_QHASH qHash(const SchemaResolver::ObjectCacheKey& key)
{
    return qHash(key.type) ^ qHash(key.db) ^ qHash(key.value1) ^ qHash(key.value2) ^ qHash(key.value3) ^ qHash(key.skipSystemObj);
}

int operator==(const SchemaResolver::ObjectCacheKey& k1, const SchemaResolver::ObjectCacheKey& k2)
{
    return (
        k1.type == k2.type &&
        k1.db == k2.db &&
        k1.value1 == k2.value1 &&
        k1.value2 == k2.value2 &&
        k1.value3 == k2.value3 &&
        k1.skipSystemObj == k2.skipSystemObj
    );
}
