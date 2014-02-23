#include "completionhelper.h"
#include "completioncomparer.h"
#include "db/db.h"
#include "parser/keywords.h"
#include "parser/parser.h"
#include "parser/lexer.h"
#include "parser/ast/sqlitecreatetable.h"
#include "parser/ast/sqlitecreatetrigger.h"
#include "dbattacher.h"
#include "utils.h"
#include "utils_sql.h"
#include "db/dbmanager.h"
#include "sqlitestudio.h"
#include <QStringList>
#include <QDebug>

QStringList sqlite3Pragmas;
QStringList sqlite2Pragmas;
QStringList sqlite3Functions;
QStringList sqlite2Functions;

CompletionHelper::Results CompletionHelper::getExpectedTokens(const QString &sql, Db* db)
{
    if (!db)
        return Results();

    return getExpectedTokens(sql, sql.length(), db);
}

CompletionHelper::Results CompletionHelper::getExpectedTokens(const QString& sql, qint32 cursorPos, Db* db)
{
    if (!db)
        return Results();

    // TODO smarter suggestions for UPDATE, DELETE and INSERT, just like for SELECT.

    CompletionHelper helper(sql, cursorPos, db);
    return helper.getExpectedTokens();
}

void CompletionHelper::init()
{
    sqlite3Pragmas << "auto_vacuum" << "automatic_index" << "busy_timeout" << "cache_size"
                   << "case_sensitive_like" << "checkpoint_fullfsync" << "collation_list"
                   << "compile_options" << "count_changes" << "data_store_directory"
                   << "database_list" << "default_cache_size" << "empty_result_callbacks"
                   << "encoding" << "foreign_key_check" << "foreign_key_list" << "foreign_keys"
                   << "freelist_count" << "full_column_names" << "fullfsync"
                   << "ignore_check_constraints" << "incremental_vacuum" << "index_info"
                   << "index_list" << "integrity_check" << "journal_mode" << "journal_size_limit"
                   << "legacy_file_format" << "locking_mode" << "max_page_count" << "page_count"
                   << "page_size" << "quick_check" << "read_uncommitted" << "recursive_triggers"
                   << "reverse_unordered_selects" << "schema_version" << "secure_delete"
                   << "short_column_names" << "shrink_memory" << "synchronous" << "table_info"
                   << "temp_store" << "temp_store_directory" << "user_version"
                   << "wal_autocheckpoint" << "wal_checkpoint" << "writable_schema";

    sqlite2Pragmas << "cache_size" << "count_changes" << "database_list" << "default_cache_size"
                   << "default_synchronous" << "default_temp_store" << "empty_result_callbacks"
                   << "foreign_key_list" << "full_column_names" << "index_info" << "index_list"
                   << "integrity_check" << "parser_trace" << "show_datatypes" << "synchronous"
                   << "table_info" << "temp_store";

    sqlite3Functions << "avg(X)" << "count(X)" << "count(*)" << "group_concat(X)"
                     << "group_concat(X,Y)" << "max(X)" << "min(X)" << "sum(X)" << "total(X)"
                     << "abs(X)" << "changes()" << "char(X1,X2,...,XN)" << "coalesce(X,Y,...)"
                     << "glob(X,Y)" << "ifnull(X,Y)" << "instr(X,Y)" << "hex(X)"
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
                     << "strftime(format,timestr,mod,mod,...)";

    sqlite2Functions << "abs(X)" << "coalesce(X,Y,...)" << "glob(X,Y)" << "ifnull(X,Y)"
                     << "last_insert_rowid()" << "length(X)" << "like(X,Y)" << "lower(X)"
                     << "max(X,Y,...)" << "min(X,Y,...)" << "nullif(X,Y)" << "random(*)"
                     << "round(X,)" << "round(X,Y)" << "soundex(X)" << "sqlite_version(*)"
                     << "substr(X,Y,Z)" << "typeof(X)" << "upper(X)" << "avg(X)" << "count(X)"
                     << "count(*)" << "max(X)" << "min(X)" << "sum(X)";

    sqlite2Pragmas.sort();
    sqlite3Pragmas.sort();
    sqlite2Functions.sort();
    sqlite3Functions.sort();
}

CompletionHelper::CompletionHelper(const QString &sql, quint32 cursorPos, Db* db)
    : db(db), cursorPosition(cursorPos), fullSql(sql)
{
    schemaResolver = new SchemaResolver(db);
    selectResolver = new SelectResolver(db, fullSql);
    selectResolver->ignoreInvalidNames = true;
    dbAttacher = new DbAttacher(db);
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
    // Get SQL up to the current cursor position.
    QString adjustedSql = fullSql.mid(0, cursorPosition);

    // If asked for completion when being in the middle of keyword or ID,
    // then remove that unfinished keyword/ID from sql and put it into
    // the final filter - to be used at the end of this method.
    QString finalFilter = QString::null;
    bool wrappedFilter = false;
    adjustedSql = removeStartedToken(adjustedSql, finalFilter, wrappedFilter);

    // Parse SQL up to cursor position, get accepted tokens and tokens that were parsed.
    Parser parser(db->getDialect());
    TokenList tokens = parser.getNextTokenCandidates(adjustedSql);
    TokenList parsedTokens = parser.getParsedTokens();

    // Parse the full sql in regular mode to extract query statement
    // for the results comparer and table-alias mapping.
    parseFullSql();

    // Handle transparent db attaching
    attachDatabases();

    // Get previous ID tokens (db and table) if any
    extractPreviousIdTokens(parsedTokens);

    // Now, that we have parsed query, we can extract some useful information
    // depending on the type of query we have.
    extractQueryAdditionalInfo();

    // Convert accepted tokens to expected tokens
    QList<ExpectedTokenPtr> results;
    foreach (TokenPtr token, tokens)
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
            results += getExpectedToken(ExpectedToken::KEYWORD, token->value);
            break;
        case Token::CTX_NEW_KW:
        {
            if (context == Context::CREATE_TRIGGER)
                results += getExpectedToken(ExpectedToken::TABLE, "new", QString::null, tr("New row reference"), 1);

            break;
        }
        case Token::CTX_OLD_KW:
        {
            if (context == Context::CREATE_TRIGGER)
                results += getExpectedToken(ExpectedToken::TABLE, "old", QString::null, tr("Old row reference"), 1);

            break;
        }
        case Token::CTX_TABLE_NEW:
            results += getExpectedToken(ExpectedToken::NO_VALUE, QString::null, QString::null, tr("New table name"));
            break;
        case Token::CTX_INDEX_NEW:
            results += getExpectedToken(ExpectedToken::NO_VALUE, QString::null, QString::null, tr("New index name"));
            break;
        case Token::CTX_VIEW_NEW:
            results += getExpectedToken(ExpectedToken::NO_VALUE, QString::null, QString::null, tr("New view name"));
            break;
        case Token::CTX_TRIGGER_NEW:
            results += getExpectedToken(ExpectedToken::NO_VALUE, QString::null, QString::null, tr("New trigger name"));
            break;
        case Token::CTX_ALIAS:
            results += getExpectedToken(ExpectedToken::NO_VALUE, QString::null, QString::null, tr("Table or column alias"));
            break;
        case Token::CTX_TRANSACTION:
            results += getExpectedToken(ExpectedToken::NO_VALUE, QString::null, QString::null, tr("transaction name"));
            break;
        case Token::CTX_COLUMN_NEW:
            results += getExpectedToken(ExpectedToken::NO_VALUE, QString::null, QString::null, tr("New column name"));
            break;
        case Token::CTX_COLUMN_TYPE:
            results += getExpectedToken(ExpectedToken::NO_VALUE, QString::null, QString::null, tr("Column data type"));
            break;
        case Token::CTX_CONSTRAINT:
            results += getExpectedToken(ExpectedToken::NO_VALUE, QString::null, QString::null, tr("Constraint name"));
            break;
        case Token::CTX_FK_MATCH:
        {
            foreach (QString kw, getFkMatchKeywords())
                results += getExpectedToken(ExpectedToken::KEYWORD, kw);

            break;
        }
        case Token::CTX_PRAGMA:
            results += getPragmas(db->getDialect());
            break;
        case Token::CTX_ERROR_MESSAGE:
            results += getExpectedToken(ExpectedToken::NO_VALUE, QString::null, QString::null, tr("Error message"));
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
            results += getFunctions(db->getDialect());
            break;
        }
        case Token::CTX_COLLATION:
        {
            if (db->getDialect() == Dialect::Sqlite2)
            {
                // SQLite 2 doesn't really support collation. It has collations
                // in grammar, but doesn't make use of them. There's no list
                // of collations to be suggested.
                results += getExpectedToken(ExpectedToken::NO_VALUE, QString::null, QString::null, tr("Collation name"));
            }
            else
            {
                results += getCollations();
            }
            break;
        }
        case Token::CTX_JOIN_OPTS:
        {
            foreach (QString joinKw, getJoinKeywords())
                results += getExpectedToken(ExpectedToken::KEYWORD, joinKw);
            break;
        }
        case Token::OTHER:
            results += getExpectedToken(ExpectedToken::OTHER, QString::null, QString::null, tr("Any word"));
            break;
        case Token::STRING:
            results += getExpectedToken(ExpectedToken::STRING);
            break;
        case Token::FLOAT:
            results += getExpectedToken(ExpectedToken::NUMBER);
            break;
        case Token::INTEGER:
            results += getExpectedToken(ExpectedToken::NUMBER);
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
            results += getExpectedToken(ExpectedToken::BLOB);
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

QList<ExpectedTokenPtr> CompletionHelper::getTables()
{
    QString dbName;
    if (previousId)
    {
        dbName = previousId->value;
        QStringList databases = schemaResolver->getDatabases().toList();
        databases += DBLIST->getDbNames();
        if (!databases.contains(dbName, Qt::CaseInsensitive))
            return QList<ExpectedTokenPtr>(); // if db is not on the set, then getObjects() would return empty list anyway;
    }

    QList<ExpectedTokenPtr> tables = getObjects(ExpectedToken::TABLE);
    tables += getExpectedToken(ExpectedToken::TABLE, "sqlite_master", dbName);
    tables += getExpectedToken(ExpectedToken::TABLE, "sqlite_temp_master", dbName);
    return tables;
}

QList<ExpectedTokenPtr> CompletionHelper::getIndexes()
{
    return getObjects(ExpectedToken::INDEX);
}

QList<ExpectedTokenPtr> CompletionHelper::getTriggers()
{
    return getObjects(ExpectedToken::TRIGGER);
}

QList<ExpectedTokenPtr> CompletionHelper::getViews()
{
    return getObjects(ExpectedToken::VIEW);
}

QList<ExpectedTokenPtr> CompletionHelper::getDatabases()
{
    QList<ExpectedTokenPtr> results;

    results += getExpectedToken(ExpectedToken::DATABASE, "main", "main", tr("Default database"));
    results += getExpectedToken(ExpectedToken::DATABASE, "temp", "temp", tr("Temporary objects database"));

    QSet<QString> databases = schemaResolver->getDatabases();
    foreach (QString dbName, databases)
    {
        if (dbAttacher->getDbNameToAttach().containsRight(dbName, Qt::CaseInsensitive))
            continue;

        results += getExpectedToken(ExpectedToken::DATABASE, dbName);
    }

    Dialect dialect = db->getDialect();

    foreach (Db* otherDb, DBLIST->getDbList())
    {
        if (otherDb->getDialect() != dialect)
            continue;

        results += getExpectedToken(ExpectedToken::DATABASE, otherDb->getName());
    }

    return results;
}

QList<ExpectedTokenPtr> CompletionHelper::getObjects(ExpectedToken::Type type)
{
    QString dbName;
    QString originalDbName;
    if (previousId)
    {
        dbName = translateDatabase(previousId->value);
        originalDbName = previousId->value;
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
    foreach (QString object, schemaResolver->getObjects(dbName, typeStr))
        results << getExpectedToken(type, object, originalDbName);

    return results;
}

QList<ExpectedTokenPtr> CompletionHelper::getColumns()
{
    QList<ExpectedTokenPtr> results;
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

    // No db or table provided. For each column its table is remembered,
    // so in case some column repeats in more than one table, then we need
    // to add prefix for the completion proposal.
    QHash<QString,QStringList> columnList;

    // Getting all tables for main db. If any column repeats in many tables,
    // then tables are stored as a list for the same column.
    foreach (QString table, schemaResolver->getTables(QString::null))
        foreach (QString column, schemaResolver->getTableColumns(table))
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
    foreach (SelectResolver::Table resolverTable, selectAvailableTables + parentSelectAvailableTables)
    {
        // This method is called only when collecting columns of tables in "main" database.
        // If here we have resolved table from other database, we don't compare it.
        if (!resolverTable.database.isNull() && resolverTable.database.toLower() != "main")
            continue;

        availableTableNames += resolverTable.table;
    }

    int availableTableCount = 0;
    foreach (QString availTable, availableTableNames)
        if (tables.contains(availTable))
            availableTableCount++;

    foreach (QString table, tables)
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
                foreach (prefix, tableToAlias[prefix])
                {
                    label = prefix+" = "+table;
                    results << getExpectedToken(ExpectedToken::COLUMN, column, table, label, prefix);
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

    // CREATE TRIGGER has a special "old" and "new" keywords as aliases for deleted/inserted/updated rows.
    // They should refer to a table that the trigger is created for.
    QString tableLower = table;
    if (context == Context::CREATE_TRIGGER && (tableLower == "old" || tableLower == "new"))
    {
        SqliteCreateTriggerPtr createTrigger = parsedQuery.dynamicCast<SqliteCreateTrigger>();
        if (createTrigger && !createTrigger->table.isNull())
        {
            table = createTrigger->table;
            label = table;
        }
    }

    // Get columns for given table in main db.
    foreach (const QString& column, schemaResolver->getTableColumns(dbName, table))
        results << getExpectedToken(ExpectedToken::COLUMN, column, table, label);

    return results;
}

QList<ExpectedTokenPtr> CompletionHelper::getColumns(const QString &prefixDb, const QString &prefixTable)
{
    QList<ExpectedTokenPtr> results;

    // Get columns for given table in given db.
    QString context = prefixDb+"."+prefixTable;
    foreach (const QString& column, schemaResolver->getTableColumns(translateDatabase(prefixDb), prefixTable))
        results << getExpectedToken(ExpectedToken::COLUMN, column, context);

    return results;
}

QList<ExpectedTokenPtr> CompletionHelper::getFavoredColumns(const QList<ExpectedTokenPtr>& resultsSoFar)
{
    // Prepare list that doesn't create duplicates with the results we already have.
    // Since results so far have more chance to provide context into, we will keep the original ones
    // from results so far and avoid adding then from favored list.
    QStringList columnsToAdd = favoredColumnNames;
    foreach (const ExpectedTokenPtr& token, resultsSoFar)
    {
        if (token->prefix.isNull() && columnsToAdd.contains(token->value))
            columnsToAdd.removeOne(token->value);
    }

    QString ctxInfo;
    if (context == Context::CREATE_TABLE && parsedQuery)
        ctxInfo = parsedQuery.dynamicCast<SqliteCreateTable>()->table;

    QList<ExpectedTokenPtr> results;
    foreach (const QString& column, columnsToAdd)
        results << getExpectedToken(ExpectedToken::COLUMN, column, ctxInfo);

    return results;
}

QList<ExpectedTokenPtr> CompletionHelper::getFunctions(Dialect dialect)
{
    // TODO to do later - make function completion more verbose,
    // like what are arguments of the function, etc.
    QStringList functions;
    if (dialect == Dialect::Sqlite2)
        functions = sqlite2Functions;
    else
        functions = sqlite3Functions;

    QList<ExpectedTokenPtr> expectedTokens;
    foreach (QString function, functions)
        expectedTokens += getExpectedToken(ExpectedToken::FUNCTION, function);

    return expectedTokens;
}

QList<ExpectedTokenPtr> CompletionHelper::getPragmas(Dialect dialect)
{
    QStringList pragmas;
    if (dialect == Dialect::Sqlite2)
        pragmas = sqlite2Pragmas;
    else
        pragmas = sqlite3Pragmas;

    QList<ExpectedTokenPtr> expectedTokens;
    foreach (QString pragma, pragmas)
        expectedTokens += getExpectedToken(ExpectedToken::PRAGMA, pragma);

    return expectedTokens;
}

QList<ExpectedTokenPtr> CompletionHelper::getCollations()
{
    SqlResultsPtr results = db->exec("PRAGMA collation_list;");
    if (results->isError())
    {
        qWarning() << "Got error when trying to get collation_list: "
                   << results->getErrorText();
    }
    QList<ExpectedTokenPtr> expectedTokens;
    foreach (SqlResultsRowPtr row, results->getAll())
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

    Parser parser(db->getDialect());
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

QString CompletionHelper::removeStartedToken(const QString& adjustedSql, QString& finalFilter, bool& wrappedFilter)
{
    QString result = adjustedSql;

    Lexer lexer(db->getDialect());
    TokenList tokens = lexer.tokenize(adjustedSql);
    if (tokens.size() == 0)
        return result;

    TokenPtr lastToken = tokens.last();

    if (isFilterType(lastToken->type))
    {
        result = Lexer::detokenize(tokens.mid(0, tokens.size()-1));
        finalFilter = lastToken->value;

        if (finalFilter.length() > 0 && isWrapperChar(finalFilter[0], db->getDialect()))
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
    foreach (TokenPtr token, tokens)
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
    foreach (TokenPtr token, tokens)
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
    QSet<ExpectedTokenPtr> set = resultsSoFar.toSet();
    resultsSoFar = set.toList();
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
    Dialect dialect = db->getDialect();

    Parser parser(dialect);
    QString sql = fullSql;

    // Selecting query at cursor position
    QString query = getQueryWithPosition(sql, cursorPosition, dialect);

    // Token list of the query. Also useful, not only parsed query.
    queryTokens = Lexer::tokenize(query, dialect);
    queryTokens.trim();

    // Completing query
    if (!query.trimmed().endsWith(";"))
        query += ";";

    // Parsing query
    if (parser.parse(query, true) && !parser.getQueries().isEmpty())
        parsedQuery = parser.getQueries().first();
}

void CompletionHelper::sort(QList<ExpectedTokenPtr> &resultsSoFar)
{
    CompletionComparer comparer(this);
    qSort(resultsSoFar.begin(), resultsSoFar.end(), comparer);
}

void CompletionHelper::extractPreviousIdTokens(const TokenList &parsedTokens)
{
    Dialect dialect = db->getDialect();

    // The previous ID token (if any) is being used in
    // getExpectedToken() and it's always the same token,
    // so here we find it just once and reuse it.
    previousId = stripObjName(getPreviousDbOrTable(parsedTokens), dialect);

    // In case of column context we need to know if there was
    // up to two ID tokens before. If we had one above,
    // then we check for one more here.
    twoIdsBack.clear();
    if (previousId)
    {
        int idx = parsedTokens.indexOf(previousId);
        TokenList parsedTokensSubSet = parsedTokens.mid(0, idx);
        twoIdsBack = stripObjName(getPreviousDbOrTable(parsedTokensSubSet), dialect);
    }
}

void CompletionHelper::extractQueryAdditionalInfo()
{
    if (extractSelectCore())
    {
        extractSelectAvailableColumnsAndTables();
        extractTableAliasMap();
        removeDuplicates(parentSelectAvailableColumns);
        detectSelectContext();
    }
    else if (isInUpdateColumn())
    {
        context = Context::UPDATE_COLUMN;
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
}

void CompletionHelper::detectSelectContext()
{
    if (isInResCols(currentSelectCore))
    {
        context = Context::SELECT_RESULT_COLUMN;
        return;
    }

    if (isInFromClause(currentSelectCore))
    {
        context = Context::SELECT_FROM;
        return;
    }
}

bool CompletionHelper::isInResCols(SqliteSelect::Core* core)
{
    if (core->tokens.isEmpty())
    {
        qWarning() << "Empty tokens of select core in call to CompletionHelper::isInResCols().";
        return false;
    }

    if (cursorPosition <= core->tokens.first()->end)
        return false; // still in SELECT token

    // The end index is not the last token in results columns, but first token in any statement,
    // that is not results columns. If there's no other statement, then it means that cursor
    // is in results column, cause there's nothing after them.
    qint64 endIdx = -1;
    const char* tokensMapKeys[] = {"from", "where_opt", "groupby_opt", "having_opt"};
    for (const char* tokensMapKey : tokensMapKeys)
    {
        if (core->tokensMap[tokensMapKey].size() > 0)
        {
            endIdx = core->tokensMap[tokensMapKey].first()->start;
            break;
        }
    }

    if (endIdx == -1)
        return true;

    if (cursorPosition >= endIdx)
        return false;

    return true;
}

bool CompletionHelper::isInFromClause(SqliteSelect::Core *core)
{
    if (core->tokens.isEmpty())
    {
        qWarning() << "Empty tokens of select core in call to CompletionHelper::isInFromClause().";
        return false;
    }

    TokenPtr from = core->tokensMap["from"].find(Token::KEYWORD, "FROM", Qt::CaseInsensitive);
    if (!from)
    {
        // No from, so no FROM context.
        return false;
    }

    if (cursorPosition <= from->end)
    {
        // Still in FROM word
        return false;
    }

    qint64 endIdx = -1;
    const char* tokensMapKeys[] = {"where_opt", "groupby_opt", "having_opt"};
    for (const char* tokensMapKey : tokensMapKeys)
    {
        if (core->tokensMap[tokensMapKey].size() > 0)
        {
            endIdx = core->tokensMap[tokensMapKey].first()->start;
            break;
        }
    }

    if (endIdx == -1)
        return true;

    if (cursorPosition >= endIdx)
        return false;

    return true;
}

bool CompletionHelper::isInUpdateColumn()
{
    // TODO need to check if we're not in any subquery, because this disqualifies update column context
    // and raises opportunity for any kind of select context.

    if (!parsedQuery)
        return false;

    if (parsedQuery->queryType != SqliteQueryType::Update)
        return false;

    // We're looking for curPos - 1, because tokens indexing ends in the same, with -1 index.
    TokenPtr token = parsedQuery->tokens.atCursorPosition(cursorPosition - 1);
    if (!token)
        return false;

    return parsedQuery->tokensMap["setlist"].contains(token);
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

bool CompletionHelper::testQueryToken(int tokenPosition, Token::Type type, const QString& value, Qt::CaseSensitivity cs)
{
    if (tokenPosition >= queryTokens.size())
        return false;

    if (tokenPosition < 0)
        return 0;

    TokenPtr token = queryTokens[tokenPosition];
    return (token->type == type && token->value.compare(value, cs) == 0);
}

bool CompletionHelper::extractSelectCore()
{
    if (!parsedQuery)
        return false;

    // Finding in which statement the cursor is positioned
    // We're looking for curPos - 1, because tokens indexing ends in the same, with -1 index.
    SqliteStatement* stmt = parsedQuery->findStatementWithPosition(cursorPosition - 1);

    // Now going up in statements tree in order to find first select core
    while (stmt && !dynamic_cast<SqliteSelect::Core*>(stmt))
        stmt = stmt->parentStatement();

    if (stmt && dynamic_cast<SqliteSelect::Core*>(stmt))
    {
        // We found our select core
        currentSelectCore = dynamic_cast<SqliteSelect::Core*>(stmt);
        return true;
    }

    return false;
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

void CompletionHelper::extractTableAliasMap()
{
    foreach (SelectResolver::Column column, selectAvailableColumns)
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
    foreach (SelectResolver::Column column, parentSelectAvailableColumns)
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
    foreach (SqliteCreateTable::Column* col, createTable->columns)
        favoredColumnNames << col->name;
}

QList<ExpectedTokenPtr> CompletionHelper::Results::filtered()
{
    QList<ExpectedTokenPtr> tokens = expectedTokens;
    applyFilter(tokens, partialToken);
    return tokens;
}
