#include "completionhelper.h"
#include "completioncomparer.h"
#include "db/db.h"
#include "parser/ast/sqlitedelete.h"
#include "parser/ast/sqliteinsert.h"
#include "parser/ast/sqliteupdate.h"
#include "parser/keywords.h"
#include "parser/parser.h"
#include "parser/lexer.h"
#include "parser/ast/sqlitecreatetable.h"
#include "parser/ast/sqlitecreatetrigger.h"
#include "dbattacher.h"
#include "common/utils.h"
#include "common/utils_sql.h"
#include "services/dbmanager.h"
#include "db/dbsqlite3.h"
#include <QStringList>
#include <QDebug>

QStringList sqlite3Pragmas;
QStringList sqlite3Functions;

bool CompletionHelper::enableLemonDebug = false;

CompletionHelper::CompletionHelper(const QString &sql, Db* db)
    : CompletionHelper(sql, sql.length(), db)
{
}

CompletionHelper::CompletionHelper(const QString &sql, quint32 cursorPos, Db* db)
    : db(db), cursorPosition(cursorPos), fullSql(sql)
{
    schemaResolver = new SchemaResolver(db);
    selectResolver = new SelectResolver(db, fullSql);
    selectResolver->ignoreInvalidNames = true;
    dbAttacher = SQLITESTUDIO->createDbAttacher(db);
}

void CompletionHelper::init()
{
    Db* db = new DbSqlite3("CompletionHelper::init()", ":memory:", {{DB_PURE_INIT, true}});
    if (!db->open())
        qWarning() << "Could not open memory db for initializing function list:" << db->getErrorText();

    initFunctions(db);
    initPragmas(db);
    delete db;

    sqlite3Pragmas.sort();
    sqlite3Functions.sort();
}

CompletionHelper::~CompletionHelper()
{
    if (schemaResolver)
    {
        delete schemaResolver;
        schemaResolver = nullptr;
    }

    if (selectResolver)
    {
        delete selectResolver;
        selectResolver = nullptr;
    }

    if (dbAttacher)
    {
        delete dbAttacher;
        dbAttacher = nullptr;
    }
}

CompletionHelper::Results CompletionHelper::getExpectedTokens()
{
    if (!db || !db->isValid())
        return Results();

    // Get SQL up to the current cursor position.
    QString adjustedSql = fullSql.mid(0, cursorPosition);

    // If asked for completion when being in the middle of keyword or ID,
    // then remove that unfinished keyword/ID from sql and put it into
    // the final filter - to be used at the end of this method.
    QString finalFilter = QString();
    bool wrappedFilter = false;
    adjustedSql = removeStartedToken(adjustedSql, finalFilter, wrappedFilter);

    // Parse SQL up to cursor position, get accepted tokens and tokens that were parsed.
    Parser parser;
    TokenList tokens = parser.getNextTokenCandidates(adjustedSql);
    TokenList parsedTokens = parser.getParsedTokens();

    // Parse the full sql in regular mode to extract query statement
    // for the results comparer and table-alias mapping.
    parseFullSql();

    // Collect used db names in original query (before using attach names)
    collectOtherDatabases();

    // Handle transparent db attaching
    attachDatabases();

    // Get previous ID tokens (db and table) if any
    extractPreviousIdTokens(parsedTokens);

    // Now, that we have parsed query, we can extract some useful information
    // depending on the type of query we have.
    extractQueryAdditionalInfo();

    // Convert accepted tokens to expected tokens
    QList<ExpectedTokenPtr> results;
    for (TokenPtr token : tokens)
        results += getExpectedTokens(token);

    // Filter redundant tokens from results
    filterContextKeywords(results, tokens);
    filterOtherId(results, tokens);
    filterDuplicates(results);

    // ...and sort the output.
    sort(results);

    // Detach any databases attached for the completer needs
    detachDatabases();

    Results complexResult;
    complexResult.expectedTokens = results;
    complexResult.partialToken = finalFilter;
    complexResult.wrappedToken = wrappedFilter;
    return complexResult;
}

QList<ExpectedTokenPtr> CompletionHelper::getExpectedTokens(TokenPtr token)
{
    QList<ExpectedTokenPtr> results;

    // Initial conditions
    if (previousId)
    {
        if (!token->isDbObjectType())
            return results;

        if (twoIdsBack && token->type != Token::CTX_COLUMN)
            return results;
    }

    // Main routines
    switch (token->type)
    {
        case Token::CTX_ROWID_KW:
        case Token::CTX_STRICT_KW:
            results += getExpectedToken(ExpectedToken::KEYWORD, token->value);
            break;
        case Token::CTX_NEW_KW:
        {
            if (context == Context::CREATE_TRIGGER)
                results += getExpectedToken(ExpectedToken::TABLE, "new", QString(), tr("New row reference"), 1);

            break;
        }
        case Token::CTX_OLD_KW:
        {
            if (context == Context::CREATE_TRIGGER)
                results += getExpectedToken(ExpectedToken::TABLE, "old", QString(), tr("Old row reference"), 1);

            break;
        }
        case Token::CTX_TABLE_NEW:
            results += getExpectedToken(ExpectedToken::NO_VALUE, QString(), QString(), tr("New table name"));
            break;
        case Token::CTX_INDEX_NEW:
            results += getExpectedToken(ExpectedToken::NO_VALUE, QString(), QString(), tr("New index name"));
            break;
        case Token::CTX_VIEW_NEW:
            results += getExpectedToken(ExpectedToken::NO_VALUE, QString(), QString(), tr("New view name"));
            break;
        case Token::CTX_TRIGGER_NEW:
            results += getExpectedToken(ExpectedToken::NO_VALUE, QString(), QString(), tr("New trigger name"));
            break;
        case Token::CTX_ALIAS:
            results += getExpectedToken(ExpectedToken::NO_VALUE, QString(), QString(), tr("Table or column alias"));
            break;
        case Token::CTX_TRANSACTION:
            results += getExpectedToken(ExpectedToken::NO_VALUE, QString(), QString(), tr("transaction name"));
            break;
        case Token::CTX_COLUMN_NEW:
            results += getExpectedToken(ExpectedToken::NO_VALUE, QString(), QString(), tr("New column name"));
            break;
        case Token::CTX_COLUMN_TYPE:
            results += getExpectedToken(ExpectedToken::NO_VALUE, QString(), QString(), tr("Column data type"));
            break;
        case Token::CTX_CONSTRAINT:
            results += getExpectedToken(ExpectedToken::NO_VALUE, QString(), QString(), tr("Constraint name"));
            break;
        case Token::CTX_FK_MATCH:
        {
            for (QString kw : getFkMatchKeywords())
                results += getExpectedToken(ExpectedToken::KEYWORD, kw);

            break;
        }
        case Token::CTX_PRAGMA:
            results += getPragmas();
            break;
        case Token::CTX_ERROR_MESSAGE:
            results += getExpectedToken(ExpectedToken::NO_VALUE, QString(), QString(), tr("Error message"));
            break;
        case Token::CTX_COLUMN:
        {
            results += getColumns();
            break;
        }
        case Token::CTX_TABLE:
        {
            results += getTables();
            break;
        }
        case Token::CTX_INDEX:
        {
            results += getIndexes();
            break;
        }
        case Token::CTX_TRIGGER:
        {
            results += getTriggers();
            break;
        }
        case Token::CTX_VIEW:
        {
            results += getViews();
            break;
        }
        case Token::CTX_DATABASE:
        {
            results += getDatabases();
            break;
        }
        case Token::CTX_FUNCTION:
        {
            results += getFunctions(db);
            break;
        }
        case Token::CTX_COLLATION:
        {
            results += getCollations();
            break;
        }
        case Token::CTX_JOIN_OPTS:
        {
            for (QString joinKw : getJoinKeywords())
                results += getExpectedToken(ExpectedToken::KEYWORD, joinKw);
            break;
        }
        case Token::OTHER:
            results += getExpectedToken(ExpectedToken::OTHER, QString(), QString(), tr("Any word"));
            break;
        case Token::STRING:
            results += getExpectedToken(ExpectedToken::STRING, "''", QString(), tr("String"));
            break;
        case Token::FLOAT:
            results += getExpectedToken(ExpectedToken::NUMBER, QString(), QString(), tr("Number"));
            break;
        case Token::INTEGER:
            results += getExpectedToken(ExpectedToken::NUMBER, QString(), QString(), tr("Number"));
            break;
        case Token::OPERATOR:
            results += getExpectedToken(ExpectedToken::OPERATOR, token->value);
            break;
        case Token::PAR_LEFT:
            results += getExpectedToken(ExpectedToken::OPERATOR, "(");
            break;
        case Token::PAR_RIGHT:
            results += getExpectedToken(ExpectedToken::OPERATOR, ")");
            break;
        case Token::BLOB:
            results += getExpectedToken(ExpectedToken::BLOB, "X''", QString(), tr("BLOB literal"));
            break;
        case Token::KEYWORD:
            results += getExpectedToken(ExpectedToken::KEYWORD, token->value);
            break;
        case Token::INVALID:
            // No-op
            break;
        case Token::BIND_PARAM:
            // No-op
            break;
        case Token::SPACE:
            // No-op
            break;
        case Token::COMMENT:
            // No-op
            break;
    }

    return results;
}

ExpectedTokenPtr CompletionHelper::getExpectedToken(ExpectedToken::Type type)
{
    ExpectedToken* token = new ExpectedToken();
    token->type = type;
    return ExpectedTokenPtr(token);
}

ExpectedTokenPtr CompletionHelper::getExpectedToken(ExpectedToken::Type type, const QString &value)
{
    ExpectedTokenPtr token = getExpectedToken(type);
    token->value = value;
    return token;
}

ExpectedTokenPtr CompletionHelper::getExpectedToken(ExpectedToken::Type type, const QString &value,
                                                    int priority)
{
    ExpectedTokenPtr token = getExpectedToken(type, value);
    token->priority = priority;
    return token;
}

ExpectedTokenPtr CompletionHelper::getExpectedToken(ExpectedToken::Type type, const QString &value,
                                                    const QString& contextInfo)
{
    ExpectedTokenPtr token = getExpectedToken(type, value);
    token->contextInfo = contextInfo;
    token->label = contextInfo;
    return token;
}

ExpectedTokenPtr CompletionHelper::getExpectedToken(ExpectedToken::Type type, const QString &value,
                                                    const QString& contextInfo, int priority)
{
    ExpectedTokenPtr token = getExpectedToken(type, value, contextInfo);
    token->priority = priority;
    return token;
}

ExpectedTokenPtr CompletionHelper::getExpectedToken(ExpectedToken::Type type, const QString &value,
                                                    const QString& contextInfo,
                                                    const QString& label)
{
    ExpectedTokenPtr token = getExpectedToken(type, value, contextInfo);
    token->label = label;
    return token;
}

ExpectedTokenPtr CompletionHelper::getExpectedToken(ExpectedToken::Type type, const QString &value,
                                                    const QString& contextInfo,
                                                    const QString& label, int priority)
{
    ExpectedTokenPtr token = getExpectedToken(type, value, contextInfo, label);
    token->priority = priority;
    return token;
}

ExpectedTokenPtr CompletionHelper::getExpectedToken(ExpectedToken::Type type, const QString &value,
                                                    const QString& contextInfo,
                                                    const QString& label,
                                                    const QString& prefix)
{
    ExpectedTokenPtr token = getExpectedToken(type, value, contextInfo, label);
    token->prefix = prefix;
    return token;
}

ExpectedTokenPtr CompletionHelper::getExpectedToken(ExpectedToken::Type type, const QString &value,
                                                    const QString& contextInfo,
                                                    const QString& label,
                                                    const QString& prefix,
                                                    int priority)
{
    ExpectedTokenPtr token = getExpectedToken(type, value, contextInfo, label, prefix);
    token->priority = priority;
    return token;
}

bool CompletionHelper::validatePreviousIdForGetObjects(QString* dbName)
{
    QString localDbName;
    if (previousId)
    {
        localDbName = previousId->value;
        QStringList databases = schemaResolver->getDatabases().values();
        databases += DBLIST->getDbNames();
        if (!databases.contains(localDbName, Qt::CaseInsensitive))
            return false; // if db is not on the set, then getObjects() would return empty list anyway;

        if (dbName)
            *dbName = localDbName;
    }
    return true;
}

QList<ExpectedTokenPtr> CompletionHelper::getTables()
{
    QString dbName;
    if (!validatePreviousIdForGetObjects(&dbName))
        return QList<ExpectedTokenPtr>();

    QList<ExpectedTokenPtr> tables = getObjects(ExpectedToken::TABLE);
    for (const QString& otherDb : otherDatabasesToLookupFor)
        tables += getObjects(ExpectedToken::TABLE, otherDb);

    tables += getExpectedToken(ExpectedToken::TABLE, "sqlite_master", dbName);
    tables += getExpectedToken(ExpectedToken::TABLE, "sqlite_temp_master", dbName);
    return tables;
}

QList<ExpectedTokenPtr> CompletionHelper::getIndexes()
{
    if (!validatePreviousIdForGetObjects())
        return QList<ExpectedTokenPtr>();

    return getObjects(ExpectedToken::INDEX);
}

QList<ExpectedTokenPtr> CompletionHelper::getTriggers()
{
    if (!validatePreviousIdForGetObjects())
        return QList<ExpectedTokenPtr>();

    return getObjects(ExpectedToken::TRIGGER);
}

QList<ExpectedTokenPtr> CompletionHelper::getViews()
{
    if (!validatePreviousIdForGetObjects())
        return QList<ExpectedTokenPtr>();

    return getObjects(ExpectedToken::VIEW);
}

QList<ExpectedTokenPtr> CompletionHelper::getDatabases()
{
    QList<ExpectedTokenPtr> results;

    results += getExpectedToken(ExpectedToken::DATABASE, "main", "main", tr("Default database"));
    results += getExpectedToken(ExpectedToken::DATABASE, "temp", "temp", tr("Temporary objects database"));

    QSet<QString> databases = schemaResolver->getDatabases();
    for (QString dbName : databases)
    {
        if (dbAttacher->getDbNameToAttach().containsRight(dbName, Qt::CaseInsensitive))
            continue;

        results += getExpectedToken(ExpectedToken::DATABASE, dbName);
    }

    for (Db* otherDb : DBLIST->getValidDbList())
        results += getExpectedToken(ExpectedToken::DATABASE, otherDb->getName());

    return results;
}

QList<ExpectedTokenPtr> CompletionHelper::getObjects(ExpectedToken::Type type)
{
    if (previousId)
        return getObjects(type, previousId->value);
    else
        return getObjects(type, QString());
}

QList<ExpectedTokenPtr> CompletionHelper::getObjects(ExpectedToken::Type type, const QString& database)
{
    QString dbName;
    QString originalDbName;
    if (!database.isNull())
    {
        dbName = translateDatabase(database);
        originalDbName = database;
    }

    QString typeStr;
    switch (type)
    {
        case ExpectedToken::TABLE:
            typeStr = "table";
            break;
        case ExpectedToken::INDEX:
            typeStr = "index";
            break;
        case ExpectedToken::TRIGGER:
            typeStr = "trigger";
            break;
        case ExpectedToken::VIEW:
            typeStr = "view";
            break;
        default:
            qWarning() << "Invalid type passed to CompletionHelper::getObject().";
            return QList<ExpectedTokenPtr>();
    }

    QList<ExpectedTokenPtr> results;
    for (QString object : schemaResolver->getObjects(dbName, typeStr))
        results << getExpectedToken(type, object, originalDbName);

    return results;
}

QList<ExpectedTokenPtr> CompletionHelper::getColumns()
{
    QList<ExpectedTokenPtr> results;
    switch (context) {
        case Context::UPDATE_RETURNING:
        case Context::INSERT_RETURNING:
        case Context::DELETE_RETURNING:
            results += getExpectedToken(ExpectedToken::OPERATOR, "*", QString(), QString(), 1);
            break;
        default:
            break;
    }

    if (previousId)
    {
        if (twoIdsBack)
            results += getColumns(twoIdsBack->value, previousId->value);
        else
            results += getColumns(previousId->value);
    }
    else
    {
        results += getColumnsNoPrefix();
    }

    if (favoredColumnNames.size() > 0)
        results += getFavoredColumns(results);

    return results;
}

QList<ExpectedTokenPtr> CompletionHelper::getColumnsNoPrefix()
{
    QList<ExpectedTokenPtr> results;

    // Use available columns for other databases (transparently attached)
    QString ctx;
    for (SelectResolver::Column& column : selectAvailableColumns)
    {
        if (column.database.isNull())
            continue;

        if (column.tableAlias.isNull())
            ctx = translateDatabaseBack(column.database)+"."+column.table;
        else
            ctx = column.tableAlias+" = "+translateDatabaseBack(column.database)+"."+column.table;

        results << getExpectedToken(ExpectedToken::COLUMN, column.column, ctx);
    }

    // No db or table provided. For each column its table is remembered,
    // so in case some column repeats in more than one table, then we need
    // to add prefix for the completion proposal.
    QHash<QString,QStringList> columnList;

    // Getting all tables for main db. If any column repeats in many tables,
    // then tables are stored as a list for the same column.
    for (QString table : schemaResolver->getTables(QString()))
        for (QString column : schemaResolver->getTableColumns(table))
            columnList[column] += table;

    // Now, for each column the expected token is created.
    // If a column occured in more tables, then multiple expected tokens
    // are created to reflect all possible tables.
    QHashIterator<QString,QStringList> it(columnList);
    while (it.hasNext())
    {
        it.next();
        results << getColumnsNoPrefix(it.key(), it.value());
    }

    return results;
}

QList<ExpectedTokenPtr> CompletionHelper::getColumnsNoPrefix(const QString& column, const QStringList& tables)
{
    QList<ExpectedTokenPtr> results;

    QStringList availableTableNames;
    for (SelectResolver::Table resolverTable : selectAvailableTables + parentSelectAvailableTables)
    {
        // This method is called only when collecting columns of tables in "main" database.
        // If here we have resolved table from other database, we don't compare it.
        if (!resolverTable.database.isNull() && resolverTable.database.toLower() != "main")
            continue;

        availableTableNames += resolverTable.table;
    }

    int availableTableCount = 0;
    for (QString availTable : availableTableNames)
        if (tables.contains(availTable))
            availableTableCount++;

    for (QString table : tables)
    {
        // Table prefix is used only if there is more than one table in FROM clause
        // that has this column, or table alias was used.
        if (!currentSelectCore || (availableTableCount <= 1 && !tableToAlias.contains(table)))
            results << getExpectedToken(ExpectedToken::COLUMN, column, table);
        else
        {
            // The prefix table might need translation to an alias.
            QString prefix = table;
            QString label = table;
            if (tableToAlias.contains(prefix))
            {
                for (const QString& resolvedPrefix : tableToAlias[prefix])
                {
                    label = resolvedPrefix+" = "+table;
                    results << getExpectedToken(ExpectedToken::COLUMN, column, table, label, resolvedPrefix);
                }
            }
            else
                results << getExpectedToken(ExpectedToken::COLUMN, column, table, label, prefix);
        }
    }

    return results;
}

QList<ExpectedTokenPtr> CompletionHelper::getColumns(const QString &prefixTable)
{
    QList<ExpectedTokenPtr> results;

    QString label = prefixTable;
    QString table = prefixTable;
    QString dbName;
    if (aliasToTable.contains(prefixTable))
    {
        Table tableAndDb = aliasToTable.value(prefixTable);
        table = tableAndDb.getTable();
        dbName = tableAndDb.getDatabase();
        label = prefixTable+" = "+table;
    }

    if (!dbName.isNull())
        dbName = translateDatabase(dbName);

    // CREATE TRIGGER has a special "old" and "new" keywords as aliases for deleted/inserted/updated rows.
    // They should refer to a table that the trigger is created for.
    QString tableLower = table;
    if (context == Context::CREATE_TRIGGER && (tableLower == "old" || tableLower == "new"))
    {
        if (!createTriggerTable.isNull())
        {
            table = createTriggerTable;
            label = createTriggerTable;
        }
        else
        {
            SqliteCreateTriggerPtr createTrigger = parsedQuery.dynamicCast<SqliteCreateTrigger>();
            if (createTrigger && !createTrigger->table.isNull())
            {
                table = createTrigger->table;
                label = table;
            }
        }
    }

    // Get columns for given table in main db.
    for (const QString& column : schemaResolver->getTableColumns(dbName, table))
        results << getExpectedToken(ExpectedToken::COLUMN, column, table, label);

    return results;
}

QList<ExpectedTokenPtr> CompletionHelper::getColumns(const QString &prefixDb, const QString &prefixTable)
{
    QList<ExpectedTokenPtr> results;

    // Get columns for given table in given db.
    QString context = prefixDb+"."+prefixTable;
    for (const QString& column : schemaResolver->getTableColumns(translateDatabase(prefixDb), prefixTable))
        results << getExpectedToken(ExpectedToken::COLUMN, column, context);

    return results;
}

QList<ExpectedTokenPtr> CompletionHelper::getFavoredColumns(const QList<ExpectedTokenPtr>& resultsSoFar)
{
    // Prepare list that doesn't create duplicates with the results we already have.
    // Since results so far have more chance to provide context into, we will keep the original ones
    // from results so far and avoid adding then from favored list.
    QStringList columnsToAdd = favoredColumnNames;
    for (const ExpectedTokenPtr& token : resultsSoFar)
    {
        if (token->prefix.isNull() && columnsToAdd.contains(token->value))
            columnsToAdd.removeOne(token->value);
    }

    QString ctxInfo;
    if (context == Context::CREATE_TABLE && parsedQuery)
        ctxInfo = parsedQuery.dynamicCast<SqliteCreateTable>()->table;

    QList<ExpectedTokenPtr> results;
    for (const QString& column : columnsToAdd)
        results << getExpectedToken(ExpectedToken::COLUMN, column, ctxInfo);

    return results;
}

QList<ExpectedTokenPtr> CompletionHelper::getFunctions(Db* db)
{
    // TODO to do later - make function completion more verbose,
    // like what are arguments of the function, etc.
    QStringList functions = sqlite3Functions;

    for (FunctionManager::ScriptFunction* fn : FUNCTIONS->getScriptFunctionsForDatabase(db->getName()))
        functions << fn->toString();

    for (FunctionManager::NativeFunction* fn : FUNCTIONS->getAllNativeFunctions())
        functions << fn->toString();

    QList<ExpectedTokenPtr> expectedTokens;
    for (QString function : functions)
        expectedTokens += getExpectedToken(ExpectedToken::FUNCTION, function);

    return expectedTokens;
}

QList<ExpectedTokenPtr> CompletionHelper::getPragmas()
{
    QList<ExpectedTokenPtr> expectedTokens;
    for (QString pragma : sqlite3Pragmas)
        expectedTokens += getExpectedToken(ExpectedToken::PRAGMA, pragma);

    return expectedTokens;
}

QList<ExpectedTokenPtr> CompletionHelper::getCollations()
{
    SqlQueryPtr results = db->exec("PRAGMA collation_list;");
    if (results->isError())
    {
        qWarning() << "Got error when trying to get collation_list: "
                   << results->getErrorText();
    }
    QList<ExpectedTokenPtr> expectedTokens;
    for (SqlResultsRowPtr row : results->getAll())
        expectedTokens += getExpectedToken(ExpectedToken::COLLATION, row->value("name").toString());

    return expectedTokens;
}

TokenPtr CompletionHelper::getPreviousDbOrTable(const TokenList &parsedTokens)
{
    // First check if we even get to deal with db.table or table.column.
    // In order to do that we iterate backwards starting from the end.
    TokenPtr token;
    QListIterator<TokenPtr> it(parsedTokens);
    it.toBack();

    if (!it.hasPrevious())
        return TokenPtr(); // No tokens at all. Shouldn't really happen.

    token = it.previous();

    // Skip spaces and comments
    while ((token->type == Token::SPACE || token->type == Token::COMMENT) && it.hasPrevious())
        token = it.previous();

    // Check if first non-space and non-comment token is our dot.
    if (token->type != Token::OPERATOR || token->value != ".")
        return TokenPtr(); // Previous token is not a dot.

    // We have a dot, now let's look for another token before.
    if (!it.hasPrevious())
        return TokenPtr(); // No more tokens left in front.

    token = it.previous();

    // Skip spaces and comments
    while ((token->type == Token::SPACE || token->type == Token::COMMENT) && it.hasPrevious())
        token = it.previous();

    // Check if this token is an ID.
    if (token->type != Token::OTHER)
        return TokenPtr(); // One more token before is not an ID.

    // Okay, so now we now we have "some_id."
    // Since this method is called in known context (looking for either db or table),
    // we don't need to find out what the "some_id" is. We simple return its value.
    return token;
}

void CompletionHelper::attachDatabases()
{
    if (!parsedQuery)
        return;

    if (!dbAttacher->attachDatabases(parsedQuery))
        return;

    QString query = parsedQuery->detokenize();

    Parser parser;
    if (parser.parse(query, true) && !parser.getQueries().isEmpty())
        parsedQuery = parser.getQueries().first();
}

void CompletionHelper::detachDatabases()
{
    dbAttacher->detachDatabases();
}

QString CompletionHelper::translateDatabase(const QString& dbName)
{
    if (!dbAttacher->getDbNameToAttach().containsLeft(dbName, Qt::CaseInsensitive))
        return dbName;

    return dbAttacher->getDbNameToAttach().valueByLeft(dbName, Qt::CaseInsensitive);
}

QString CompletionHelper::translateDatabaseBack(const QString& dbName)
{
    if (!dbAttacher->getDbNameToAttach().containsRight(dbName, Qt::CaseInsensitive))
        return dbName;

    return dbAttacher->getDbNameToAttach().valueByRight(dbName, Qt::CaseInsensitive);
}

void CompletionHelper::collectOtherDatabases()
{
    otherDatabasesToLookupFor.clear();
    if (!parsedQuery)
        return;

    otherDatabasesToLookupFor = parsedQuery->getContextDatabases();
}

QString CompletionHelper::removeStartedToken(const QString& adjustedSql, QString& finalFilter, bool& wrappedFilter)
{
    QString result = adjustedSql;

    Lexer lexer;
    TokenList tokens = lexer.tokenize(adjustedSql);
    if (tokens.size() == 0)
        return result;

    TokenPtr lastToken = tokens.last();

    if (isFilterType(lastToken->type))
    {
        result = Lexer::detokenize(tokens.mid(0, tokens.size()-1));
        finalFilter = lastToken->value;

        if (finalFilter.length() > 0 && isWrapperChar(finalFilter[0]))
        {
            finalFilter = finalFilter.mid(1);
            wrappedFilter = true;
        }
    }
    return result;
}

void CompletionHelper::filterContextKeywords(QList<ExpectedTokenPtr> &resultsSoFar, const TokenList &tokens)
{
    bool wasJoinKw = false;
    bool wasFkMatchKw = false;
    for (TokenPtr token : tokens)
    {
        if (token->type == Token::CTX_JOIN_OPTS)
            wasJoinKw = true;

        if (token->type == Token::CTX_FK_MATCH)
            wasFkMatchKw = true;
    }

    if (wasJoinKw && wasFkMatchKw)
        return;

    QMutableListIterator<ExpectedTokenPtr> it(resultsSoFar);
    while (it.hasNext())
    {
        ExpectedTokenPtr token = it.next();
        if (token->type != ExpectedToken::KEYWORD)
            continue;

        if (
                (!wasJoinKw && isJoinKeyword(token->value)) ||
                (!wasFkMatchKw && isFkMatchKeyword(token->value))
            )
            it.remove();
    }
}

void CompletionHelper::filterOtherId(QList<ExpectedTokenPtr> &resultsSoFar, const TokenList &tokens)
{
    bool wasCtx = false;
    for (TokenPtr token : tokens)
    {
        switch (token->type)
        {
            case Token::CTX_COLUMN:
            case Token::CTX_TABLE:
            case Token::CTX_DATABASE:
            case Token::CTX_FUNCTION:
            case Token::CTX_COLLATION:
            case Token::CTX_INDEX:
            case Token::CTX_TRIGGER:
            case Token::CTX_VIEW:
            case Token::CTX_JOIN_OPTS:
            case Token::CTX_TABLE_NEW:
            case Token::CTX_INDEX_NEW:
            case Token::CTX_VIEW_NEW:
            case Token::CTX_TRIGGER_NEW:
            case Token::CTX_ALIAS:
            case Token::CTX_TRANSACTION:
            case Token::CTX_COLUMN_NEW:
            case Token::CTX_COLUMN_TYPE:
            case Token::CTX_CONSTRAINT:
            case Token::CTX_FK_MATCH:
            case Token::CTX_PRAGMA:
            case Token::CTX_ROWID_KW:
            case Token::CTX_STRICT_KW:
            case Token::CTX_NEW_KW:
            case Token::CTX_OLD_KW:
            case Token::CTX_ERROR_MESSAGE:
                wasCtx = true;
                break;
            default:
                break;
        }
        if (wasCtx)
            break;
    }

    if (!wasCtx)
        return;

    QMutableListIterator<ExpectedTokenPtr> it(resultsSoFar);
    while (it.hasNext())
    {
        ExpectedTokenPtr token = it.next();
        if (token->type == ExpectedToken::OTHER)
            it.remove();
    }
}

void CompletionHelper::filterDuplicates(QList<ExpectedTokenPtr>& resultsSoFar)
{
    resultsSoFar = toSet(resultsSoFar).values();
}

void CompletionHelper::applyFilter(QList<ExpectedTokenPtr>& resultsSoFar, const QString& filter)
{
    if (filter.isEmpty())
        return;

    QMutableListIterator<ExpectedTokenPtr> it(resultsSoFar);
    while (it.hasNext())
    {
        ExpectedTokenPtr token = it.next();
        if (!token->value.startsWith(filter, Qt::CaseInsensitive))
            it.remove();
    }
}

bool CompletionHelper::isFilterType(Token::Type type)
{
    switch (type)
    {
        case Token::COMMENT:
        case Token::SPACE:
        case Token::PAR_LEFT:
        case Token::PAR_RIGHT:
        case Token::OPERATOR:
            return false;
        default:
            return true;
    }
}

void CompletionHelper::parseFullSql()
{
    QString sql = fullSql;

    // Selecting query at cursor position
    QString query = getQueryWithPosition(sql, cursorPosition);

    // Token list of the query. Also useful, not only parsed query.
    queryTokens = Lexer::tokenize(query);
    queryTokens.trim();

    // Completing query
    if (!query.trimmed().endsWith(";"))
        query += ";";

    // Parsing query
    Parser parser;
    parser.setLemonDebug(enableLemonDebug);
    if (tryToParse(&parser, query))
        return;

    // Second try - handling open parenthesis for expr (which could not be handled by the grammar, because of bug #2755)
    parser.setLemonDebug(false); // avoid spamming with lemon debug
    QString truncatedSql = sql.left(cursorPosition);
    query = getQueryWithPosition(truncatedSql, cursorPosition);
    query += ");";

    if (tryToParse(&parser, query))
        return;
}

bool CompletionHelper::tryToParse(Parser* parser, const QString& query)
{
    if (parser->parse(query, true) && !parser->getQueries().isEmpty())
    {
        parsedQuery = parser->getQueries().first();
        originalParsedQuery = SqliteQueryPtr(dynamic_cast<SqliteQuery*>(parsedQuery->clone()));
        return true;
    }
    return false;
}

void CompletionHelper::sort(QList<ExpectedTokenPtr> &resultsSoFar)
{
    CompletionComparer comparer(this);
    std::sort(resultsSoFar.begin(), resultsSoFar.end(), comparer);
}

void CompletionHelper::extractPreviousIdTokens(const TokenList &parsedTokens)
{
    // The previous ID token (if any) is being used in
    // getExpectedToken() and it's always the same token,
    // so here we find it just once and reuse it.
    previousId = stripObjName(getPreviousDbOrTable(parsedTokens));

    // In case of column context we need to know if there was
    // up to two ID tokens before. If we had one above,
    // then we check for one more here.
    twoIdsBack.clear();
    if (previousId)
    {
        int idx = parsedTokens.indexOf(previousId);
        TokenList parsedTokensSubSet = parsedTokens.mid(0, idx);
        twoIdsBack = stripObjName(getPreviousDbOrTable(parsedTokensSubSet));
    }
}

void CompletionHelper::extractQueryAdditionalInfo()
{
    // Even if we're in a SELECT, it might be just a child part of INSERT INTO x SELECT ...
    // or it may be UPDATE x SET y FROM other source.
    // Because of that we extract all INSERT/DELETE/UPDATE aliases no matter if cursor is in the selectCore or not.
    extractTableAliasMapFromOtherQueries();

    extractUpdateFromColumnsAndTables();

    if (extractSelectCore())
    {
        extractSelectAvailableColumnsAndTables();
        extractSelectTableAliasMap();
        removeDuplicates(parentSelectAvailableColumns);
        detectSelectContext();
        return;
    }

    if (isInUpdateColumn())
    {
        context = Context::UPDATE_COLUMN;
    }
    else if (isInUpdateWhere())
    {
        context = Context::UPDATE_WHERE;
    }
    else if (isInDeleteWhere())
    {
        context = Context::DELETE_WHERE;
    }
    else if (isInCreateTable())
    {
        context = Context::CREATE_TABLE;
        extractCreateTableColumns();
    }
    else if (isInCreateTrigger())
    {
        context = Context::CREATE_TRIGGER;
    }
    else if (isInExpr())
    {
        context = Context::EXPR;
    }
    else if (isInInsertColumns())
    {
        context = Context::INSERT_COLUMNS;
    }
    else if (isInUpdateReturning())
    {
        context = Context::UPDATE_RETURNING;
        extractUpdateAvailableColumnsAndTables();
    }
    else if (isInInsertReturning())
    {
        context = Context::INSERT_RETURNING;
        extractInsertAvailableColumnsAndTables();
    }
    else if (isInDeleteReturning())
    {
        context = Context::DELETE_RETURNING;
        extractDeleteAvailableColumnsAndTables();
    }
}

void CompletionHelper::detectSelectContext()
{
    QStringList mapNames = {"SELECT", "distinct", "selcollist", "from", "where_opt", "groupby_opt", "having_opt", "orderby_opt", "limit_opt"};
    QList<Context> contexts = {Context::SELECT_RESULT_COLUMN, Context::SELECT_FROM, Context::SELECT_WHERE, Context::SELECT_GROUP_BY,
                              Context::SELECT_HAVING, Context::SELECT_ORDER_BY, Context::SELECT_LIMIT};

    // Assert that we have exactly 2 more map names, than defined contexts, cause we will start with 3rd map name and 1st context.
    Q_ASSERT((mapNames.size() - 2) == contexts.size());

    for (int i = 2; i < mapNames.size(); i++)
    {
        if (cursorAfterTokenMaps(currentSelectCore, mapNames.mid(0, i)) && cursorBeforeTokenMaps(currentSelectCore, mapNames.mid(i+1)))
        {
            context = contexts[i-2];
            break;
        }
    }
}

void CompletionHelper::extractTableAliasMapFromOtherQueries()
{
    if (!parsedQuery)
        return;

    SqliteQueryWithAliasedTablePtr queryWithAliasedTable = parsedQuery.dynamicCast<SqliteQueryWithAliasedTable>();
    if (!queryWithAliasedTable)
        return;

    QString database = queryWithAliasedTable->getDatabase();
    QString table = queryWithAliasedTable->getTable();
    QString alias = queryWithAliasedTable->getTableAlias();
    if (!alias.isNull() && !tableToAlias[table].contains(alias))
    {
        tableToAlias[table] += alias;
        aliasToTable[alias] = Table(database, table);
    }
}

void CompletionHelper::extractUpdateFromColumnsAndTables()
{
    if (!parsedQuery)
        return;

    SqliteUpdatePtr update = parsedQuery.objectCast<SqliteUpdate>();
    if (!update || !update->from)
        return;

    for (const SelectResolver::Table& table : selectResolver->resolveTables(update->from))
    {
        if (!table.tableAlias.isNull() && !tableToAlias[table.table].contains(table.tableAlias))
        {
            tableToAlias[table.table] += table.tableAlias;
            aliasToTable[table.tableAlias] = Table(table.database, table.table);
        }
    }
}

bool CompletionHelper::isInUpdateColumn()
{
    // We will never get here if the subquery SELECT was used anywhere in the query,
    // (and the cursor position is in that subselect), because in that case the extractSelectCore()
    // will take over the flow before it reaches here.
    return isIn(SqliteQueryType::Update, "setlist", "SET");
}

bool CompletionHelper::isInUpdateWhere()
{
    return isIn(SqliteQueryType::Update, "where_opt", "WHERE");
}

bool CompletionHelper::isInDeleteWhere()
{
    return isIn(SqliteQueryType::Delete, "where_opt", "WHERE");
}

bool CompletionHelper::isInCreateTable()
{
    if (!parsedQuery)
    {
        if (testQueryToken(0, Token::KEYWORD, "CREATE") &&
                (testQueryToken(1, Token::KEYWORD, "TABLE") ||
                 testQueryToken(2, Token::KEYWORD, "TABLE")))
        {
            return true;
        }

        return false;
    }

    if (parsedQuery->queryType != SqliteQueryType::CreateTable)
        return false;

    return true;
}

bool CompletionHelper::isInCreateTrigger()
{
    if (!parsedQuery)
    {
        if (testQueryToken(0, Token::KEYWORD, "CREATE") &&
                (testQueryToken(1, Token::KEYWORD, "TRIGGER") ||
                 testQueryToken(2, Token::KEYWORD, "TRIGGER")))
        {
            return true;
        }

        return false;
    }

    if (parsedQuery->queryType != SqliteQueryType::CreateTrigger)
        return false;

    return true;
}

bool CompletionHelper::isInUpdateReturning()
{
    return isIn(SqliteQueryType::Update, "returning", "RETURNING");
}

bool CompletionHelper::isInDeleteReturning()
{
    return isIn(SqliteQueryType::Delete, "returning", "RETURNING");
}

bool CompletionHelper::isInInsertReturning()
{
    return isIn(SqliteQueryType::Insert, "returning", "RETURNING");
}

bool CompletionHelper::isInInsertColumns()
{
    if (isIn(SqliteQueryType::Insert, "idlist_opt", QString()))
        return true;

    if (!parsedQuery)
        return false;

    if (parsedQuery->queryType != SqliteQueryType::Insert)
        return false;

    if (parsedQuery->tokensMap.contains("rp_opt"))
    {
        TokenList rpTokens = parsedQuery->tokensMap["rp_opt"];
        if (rpTokens.isEmpty())
            return parsedQuery->tokensMap["LP"][0]->start <= cursorPosition;
        else
            return rpTokens[0]->start >= cursorPosition;
    }

    return false;
}

bool CompletionHelper::isIn(SqliteQueryType queryType, const QString &tokenMapKey, const QString &prefixKeyword)
{
    if (!parsedQuery)
        return false;

    if (parsedQuery->queryType != queryType)
        return false;

    // We're looking for curPos - 1, because tokens indexing ends in the same, with -1 index.
    TokenPtr token = parsedQuery->tokens.atCursorPosition(cursorPosition - 1);
    if (!token)
        return false;

    if (parsedQuery->tokensMap[tokenMapKey].contains(token))
        return true;

    // In case cursor is just before the requested token map entry, but it is after a whitespace, then we can
    // assume, that what's coming next is our token map entry.
    if (token->isWhitespace())
    {
        int idx = parsedQuery->tokens.indexOf(token);
        if (idx < 0)
            return false;

        TokenList tokens =  parsedQuery->tokens.mid(0, idx + 1);
        tokens.trim();
        if (tokens.size() > 0 && tokens.last()->type == Token::KEYWORD && tokens.last()->value.compare(prefixKeyword, Qt::CaseInsensitive) == 0)
            return true;
    }

    return false;
}

bool CompletionHelper::isInExpr()
{
    if (!parsedQuery)
        return false;

    // Finding in which statement the cursor is positioned
    // We're looking for curPos - 1, because tokens indexing ends in the same, with -1 index.
    SqliteStatement* stmt = parsedQuery->findStatementWithPosition(cursorPosition - 1);

    // Now going up in statements tree in order to find first expr
    while (stmt && !dynamic_cast<SqliteExpr*>(stmt))
        stmt = stmt->parentStatement();

    return (stmt && dynamic_cast<SqliteExpr*>(stmt));
}

bool CompletionHelper::testQueryToken(int tokenPosition, Token::Type type, const QString& value, Qt::CaseSensitivity cs)
{
    if (tokenPosition >= queryTokens.size())
        return false;

    if (tokenPosition < 0)
        return 0;

    TokenPtr token = queryTokens[tokenPosition];
    return (token->type == type && token->value.compare(value, cs) == 0);
}

bool CompletionHelper::cursorAfterTokenMaps(SqliteStatement* stmt, const QStringList& mapNames)
{
    TokenList tokens;
    for (const QString& name : mapNames)
    {
        if (!stmt->tokensMap.contains(name) || stmt->tokensMap[name].size() == 0)
            continue;

        tokens = stmt->tokensMap[name];
        tokens.trimRight();
        if (tokens.size() == 0)
            continue;

        if (tokens.last()->end >= cursorPosition)
            return false;
    }
    return true;
}

bool CompletionHelper::cursorBeforeTokenMaps(SqliteStatement* stmt, const QStringList& mapNames)
{
    TokenList tokens;
    for (const QString& name : mapNames)
    {
        if (!stmt->tokensMap.contains(name) || stmt->tokensMap[name].size() == 0)
            continue;

        tokens = stmt->tokensMap[name];
        tokens.trimLeft();
        if (tokens.size() == 0)
            continue;

        if (tokens.first()->start < cursorPosition)
            return false;
    }
    return true;
}
QString CompletionHelper::getCreateTriggerTable() const
{
    return createTriggerTable;
}

void CompletionHelper::setCreateTriggerTable(const QString& value)
{
    createTriggerTable = value;
}

void CompletionHelper::initFunctions(Db* db)
{
    sqlite3Functions << "avg(X)" << "count(X)" << "count(*)" << "group_concat(X)"
                     << "group_concat(X,Y)" << "max(X)" << "min(X)" << "sum(X)" << "total(X)"
                     << "abs(X)" << "changes()" << "char(X1,X2,...,XN)" << "coalesce(X,Y,...)"
                     << "glob(X,Y)" << "ifnull(X,Y)" << "instr(X,Y)" << "hex(X)" << "iif(X,Y,Z)"
                     << "last_insert_rowid()" << "length(X)" << "like(X,Y)" << "like(X,Y,Z)"
                     << "load_extension(X,Y)" << "lower(X)" << "ltrim(X)" << "ltrim(X,Y)"
                     << "max(X,Y,...)" << "min(X,Y,...)" << "nullif(X,Y)" << "quote(X)"
                     << "random()" << "randomblob(N)" << "hex(randomblob(16))"
                     << "lower(hex(randomblob(16)))" << "replace(X,Y,Z)" << "round(X)"
                     << "round(X,Y)" << "rtrim(X)" << "rtrim(X,Y)" << "soundex(X)"
                     << "sqlite_compileoption_get(N)" << "sqlite_compileoption_used(X)"
                     << "sqlite_source_id()" << "sqlite_version()" << "substr(X,Y,Z)"
                     << "substr(X,Y)" << "total_changes()" << "trim(X)" << "trim(X,Y)"
                     << "typeof(X)" << "unicode(X)" << "upper(X)" << "zeroblob(N)"
                     << "date(timestr,mod,mod,...)" << "time(timestr,mod,mod,...)"
                     << "datetime(timestr,mod,mod,...)" << "julianday(timestr,mod,mod,...)"
                     << "strftime(format,timestr,mod,mod,...)" << "likelihood(X,Y)"
                     << "likely(X)" << "unlikely(X)" "row_number()" << "rank()"
                     << "dense_rank()" << "percent_rank()" << "cume_dist()" << "ntile(N)"
                     << "lag(expr)" << "lag(expr, offset)" << "lag(expr, offset, default)"
                     << "lead(expr)" << "lead(expr, offset)" << "lead(expr, offset, default)"
                     << "first_value(expr)" << "last_value(expr)" << "nth_value(expr, N)"
                     << "substring(X,Y,Z)" << "substring(X,Y)" << "unixepoch(mod,mod,...)"
                     << "printf(format,...)" << "format(format,...)";

    if (!db->isOpen())
        return;

    // Parse what we already have
    QSet<QString> handledSignatures;
    static_qstring(sigTpl, "%1_%2");
    for (QString& fn : sqlite3Functions)
    {
        int argStart = fn.lastIndexOf("(");
        int argEnd = fn.lastIndexOf(")");
        QString fnName = fn.left(argStart);
        QString args = fn.mid(argStart + 1, argEnd - argStart - 1);
        QStringList argList = args.split(",");

        int argCount = argList.size();;
        if (args.trimmed().isEmpty())
            argCount = 0;
        else if (argList.last() == "...")
            argCount = -1;

        handledSignatures << sigTpl.arg(fnName, QString::number(argCount));
    }

    // Find what is missing and add it
    static_qstring(funTpl, "%1(%2)");
    static const QStringList argSymbols = {"X", "Y", "Z", "A", "B", "C", "D", "E", "F", "G", "H", "I"};
    static const int argSymbolCnt = argSymbols.size();

    SqlQueryPtr res = db->exec("PRAGMA function_list;");
    while (res->hasNext())
    {
        SqlResultsRowPtr row = res->next();
        QVariant nargsVar = row->value("narg");
        QString fnName = row->value("name").toString();
        QString sig = sigTpl.arg(fnName, nargsVar.toString());
        if (!handledSignatures.contains(sig))
        {
            int nargs = nargsVar.toInt();
            QStringList args;
            if (nargs == -1)
                args << "...";

            for (int i = 0; i < nargs; i++)
                args << argSymbols[i % argSymbolCnt];

            sqlite3Functions << funTpl.arg(fnName, args.join(","));
            handledSignatures << sig;
        }
    }
}

void CompletionHelper::initPragmas(Db* db)
{
    if (!db->isOpen())
        return;

    SqlQueryPtr res = db->exec("PRAGMA pragma_list;");
    while (res->hasNext())
    {
        SqlResultsRowPtr row = res->next();
        QString name = row->value("name").toString();
        sqlite3Pragmas << name;
    }
}

DbAttacher* CompletionHelper::getDbAttacher() const
{
    return dbAttacher;
}

void CompletionHelper::setDbAttacher(DbAttacher* value)
{
    if (dbAttacher)
        delete dbAttacher;

    dbAttacher = value;
}


bool CompletionHelper::extractSelectCore()
{
    currentSelectCore = extractSelectCore(parsedQuery);
    originalCurrentSelectCore = extractSelectCore(originalParsedQuery);
    return (currentSelectCore != nullptr);
}

SqliteSelect::Core* CompletionHelper::extractSelectCore(SqliteQueryPtr query)
{
    if (!query)
        return nullptr;

    // Finding in which statement the cursor is positioned
    // We're looking for curPos - 1, because tokens indexing ends in the same, with -1 index.
    SqliteStatement* stmt = query->findStatementWithPosition(cursorPosition - 1);

    // Now going up in statements tree in order to find first select core
    while (stmt && !dynamic_cast<SqliteSelect::Core*>(stmt))
        stmt = stmt->parentStatement();

    if (stmt && dynamic_cast<SqliteSelect::Core*>(stmt))
    {
        // We found our select core
        return dynamic_cast<SqliteSelect::Core*>(stmt);
    }

    return nullptr;
}

void CompletionHelper::extractSelectAvailableColumnsAndTables()
{
    selectAvailableColumns = selectResolver->resolveAvailableColumns(currentSelectCore);
    selectAvailableTables = selectResolver->resolveTables(currentSelectCore);

    // Now checking for any parent select cores.
    SqliteStatement* stmt = currentSelectCore->parentStatement();
    SqliteSelect::Core* parentCore = nullptr;
    while (stmt)
    {
        while (stmt && !dynamic_cast<SqliteSelect::Core*>(stmt))
            stmt = stmt->parentStatement();

        if (!stmt || !dynamic_cast<SqliteSelect::Core*>(stmt))
            return;

        // We got another select core at higher level
        parentCore = dynamic_cast<SqliteSelect::Core*>(stmt);
        parentSelectCores += parentCore;

        // Collecting columns and tables
        parentSelectAvailableColumns += selectResolver->resolveAvailableColumns(parentCore);
        parentSelectAvailableTables += selectResolver->resolveTables(parentCore);

        // Moving on, until we're on top of the syntax tree.
        stmt = stmt->parentStatement();
    }
}

void CompletionHelper::extractInsertAvailableColumnsAndTables()
{
    auto insert = parsedQuery.dynamicCast<SqliteInsert>();
    extractAvailableColumnsAndTables(insert->database, insert->table);
}

void CompletionHelper::extractDeleteAvailableColumnsAndTables()
{
    auto del = parsedQuery.dynamicCast<SqliteDelete>();
    extractAvailableColumnsAndTables(del->database, del->table);
}

void CompletionHelper::extractUpdateAvailableColumnsAndTables()
{
    auto update = parsedQuery.dynamicCast<SqliteUpdate>();
    theFromAvailableColumns = selectResolver->resolveAvailableColumns(update->from);
    theFromAvailableTables = selectResolver->resolveTables(update->from);
}

void CompletionHelper::extractAvailableColumnsAndTables(const QString& database, const QString& table)
{
    QStringList columnNames = schemaResolver->getTableColumns(database, table);
    for (QString& colName : columnNames) {
        SelectResolver::Column column;
        column.type = SelectResolver::Column::COLUMN;
        column.database = database;
        column.table = table;
        column.column = colName;
        theFromAvailableColumns << column;
        theFromAvailableTables << column.getTable();
    }
}

void CompletionHelper::extractSelectTableAliasMap()
{
    for (SelectResolver::Column& column : selectAvailableColumns)
    {
        if (column.type != SelectResolver::Column::COLUMN)
            continue;

        if (!column.tableAlias.isNull() && !tableToAlias[column.table].contains(column.tableAlias))
        {
            tableToAlias[column.table] += column.tableAlias;
            aliasToTable[column.tableAlias] = Table(column.database, column.table);
        }
    }

    // We have sorted list of available columns from parent selects.
    // Given the above, we can extract table aliases in an order from deepest
    // to shallowest, skipping any duplicates, becase the deeper alias is mentioned,
    // the higher is its priority.
    for (SelectResolver::Column& column : parentSelectAvailableColumns)
    {
        if (column.type != SelectResolver::Column::COLUMN)
            continue;

        if (tableToAlias.contains(column.table))
            continue;

        if (!column.tableAlias.isNull() && !tableToAlias[column.table].contains(column.tableAlias))
        {
            tableToAlias[column.table] += column.tableAlias;
            aliasToTable[column.tableAlias] = Table(column.database, column.table);
        }
    }
}

void CompletionHelper::extractCreateTableColumns()
{
    if (!parsedQuery)
        return;

    SqliteCreateTablePtr createTable = parsedQuery.dynamicCast<SqliteCreateTable>();
    for (SqliteCreateTable::Column*& col : createTable->columns)
        favoredColumnNames << col->name;
}

QList<ExpectedTokenPtr> CompletionHelper::Results::filtered()
{
    QList<ExpectedTokenPtr> tokens = expectedTokens;
    applyFilter(tokens, partialToken);
    return tokens;
}
