#include "completioncomparer.h"
#include "completionhelper.h"
#include "parser/ast/sqliteselect.h"
#include <QDebug>

CompletionComparer::CompletionComparer(CompletionHelper *helper)
    : helper(helper)
{
    init();
}

bool CompletionComparer::operator ()(const ExpectedTokenPtr& token1, const ExpectedTokenPtr& token2)
{
    if (token2->type == ExpectedToken::OTHER  || token1->type == ExpectedToken::OTHER)
        qDebug() << "";

    if ((token1->priority > 0 || token2->priority > 0) && token1->priority != token2->priority)
        return token1->priority > token2->priority;

    if (token1->type != token2->type)
        return token1->type < token2->type;

    switch (token1->type)
    {
        case ExpectedToken::COLUMN:
            return compareColumns(token1, token2);
        case ExpectedToken::TABLE:
            return compareTables(token1, token2);
        case ExpectedToken::INDEX:
            return compareIndexes(token1, token2);
        case ExpectedToken::TRIGGER:
            return compareTriggers(token1, token2);
        case ExpectedToken::VIEW:
            return compareViews(token1, token2);
        case ExpectedToken::DATABASE:
            return compareDatabases(token1, token2);
        case ExpectedToken::KEYWORD:
        case ExpectedToken::FUNCTION:
        case ExpectedToken::OPERATOR:
        case ExpectedToken::PRAGMA:
            return compareValues(token1, token2);
        case ExpectedToken::COLLATION:
            return compareValues(token1, token2);
        case ExpectedToken::OTHER:
        case ExpectedToken::STRING:
        case ExpectedToken::NUMBER:
        case ExpectedToken::BLOB:
        case ExpectedToken::NO_VALUE:
            return false;
    }

    return false;
}

void CompletionComparer::init()
{
    if (helper->originalParsedQuery)
    {
        bool contextObjectsInitialized = false;
        if (helper->originalParsedQuery->queryType == SqliteQueryType::Select)
            contextObjectsInitialized = initSelect();

        if (!contextObjectsInitialized)
        {
            contextColumns = helper->originalParsedQuery->getContextColumns();
            contextTables = helper->originalParsedQuery->getContextTables();
            contextDatabases = helper->originalParsedQuery->getContextDatabases(false);
        }

        for (SelectResolver::Table table : helper->selectAvailableTables + helper->parentSelectAvailableTables)
            availableTableNames += table.table;
    }
}

bool CompletionComparer::initSelect()
{
    if (!helper->originalCurrentSelectCore)
        return false;

    // This is similar to what is done in init() itself, except here it's limited
    // to the current select core, excluding parent statement.
    contextColumns = helper->originalCurrentSelectCore->getContextColumns(false);
    contextTables = helper->originalCurrentSelectCore->getContextTables(false);
    contextDatabases = helper->originalCurrentSelectCore->getContextDatabases(false);

    for (SqliteSelect::Core*& core : helper->parentSelectCores)
    {
        parentContextColumns += core->getContextColumns(false);
        parentContextTables += core->getContextTables(false);
        parentContextDatabases += core->getContextDatabases(false);
    }

    if (helper->context == CompletionHelper::Context::SELECT_RESULT_COLUMN)
    {
        // Getting list of result columns already being selected in the query
        resultColumns = helper->selectResolver->resolve(helper->currentSelectCore);
    }

    return true;
}

bool CompletionComparer::compareColumns(const ExpectedTokenPtr& token1, const ExpectedTokenPtr& token2)
{
    if (!helper->parsedQuery)
        return compareValues(token1, token2);

    bool ok = false;
    bool result = true;
    switch (helper->context)
    {
        case CompletionHelper::Context::SELECT_WHERE:
        case CompletionHelper::Context::SELECT_GROUP_BY:
        case CompletionHelper::Context::SELECT_HAVING:
        case CompletionHelper::Context::SELECT_RESULT_COLUMN:
        case CompletionHelper::Context::SELECT_ORDER_BY:
            result = compareColumnsForSelectResCol(token1, token2, &ok);
            break;
        case CompletionHelper::Context::UPDATE_COLUMN:
        case CompletionHelper::Context::UPDATE_WHERE:
            result = compareColumnsForUpdateCol(token1, token2, &ok);
            break;
        case CompletionHelper::Context::INSERT_COLUMNS:
            result = compareColumnsForInsertCol(token1, token2, &ok);
            break;
        case CompletionHelper::Context::DELETE_WHERE:
            result = compareColumnsForDeleteCol(token1, token2, &ok);
            break;
        case CompletionHelper::Context::CREATE_TABLE:
            result = compareColumnsForCreateTable(token1, token2, &ok);
            break;
        case CompletionHelper::Context::INSERT_RETURNING:
        case CompletionHelper::Context::UPDATE_RETURNING:
        case CompletionHelper::Context::DELETE_RETURNING:
            result = compareColumnsForReturning(token1, token2, &ok);
            break;
        default:
            return compareValues(token1, token2);
    }

    if (ok)
        return result;

    result = compareByContext(token1->value, token2->value, {contextColumns, parentContextColumns}, true, &ok);
    if (ok)
        return result;

    // Context info of the column has a strong meaning when sorting (system tables are pushed to the end)
    bool firstIsSystem = token1->contextInfo.startsWith("sqlite_", Qt::CaseInsensitive);
    bool secondIsSystem = token2->contextInfo.startsWith("sqlite_", Qt::CaseInsensitive);
    if (firstIsSystem && !secondIsSystem)
        return false;

    if (!firstIsSystem && secondIsSystem)
        return true;

    return compareValues(token1->value, token2->value, true);
}

bool CompletionComparer::compareColumnsForSelectResCol(const ExpectedTokenPtr &token1, const ExpectedTokenPtr &token2, bool *result)
{
    *result = true;

    // Checking if columns are on list of columns available in FROM clause
    bool token1available = isTokenOnAvailableColumnList(token1);
    bool token2available = isTokenOnAvailableColumnList(token2);
    if (token1available && !token2available)
        return true;

    if (!token1available && token2available)
        return false;

    // Checking if columns are on list of columns available in FROM clause of any parent SELECT core
    bool token1parentAvailable = isTokenOnParentAvailableColumnList(token1);
    bool token2parentAvailable = isTokenOnParentAvailableColumnList(token2);
    if (token1parentAvailable && !token2parentAvailable)
        return true;

    if (!token1parentAvailable && token2parentAvailable)
        return false;

    // Checking if columns were already mentioned in results list.
    // It it was, it should be pushed back.
    bool token1onResCols = isTokenOnResultColumns(token1);
    bool token2onResCols = isTokenOnResultColumns(token2);
    if (token1onResCols && !token2onResCols)
        return false;

    if (!token1onResCols && token2onResCols)
        return true;

    *result = false;
    return false;
}

bool CompletionComparer::compareColumnsForUpdateCol(const ExpectedTokenPtr &token1, const ExpectedTokenPtr &token2, bool *result)
{
    *result = true;
    if (token1->contextInfo == token2->contextInfo)
        return compareValues(token1->value, token2->value);

    return compareByContext(token1->contextInfo, token2->contextInfo, contextTables, true);
}

bool CompletionComparer::compareColumnsForDeleteCol(const ExpectedTokenPtr &token1, const ExpectedTokenPtr &token2, bool *result)
{
    *result = true;
    if (token1->contextInfo == token2->contextInfo)
        return compareValues(token1->value, token2->value);

    return compareByContext(token1->contextInfo, token2->contextInfo, contextTables, true);
}

bool CompletionComparer::compareColumnsForInsertCol(const ExpectedTokenPtr &token1, const ExpectedTokenPtr &token2, bool *result)
{
    *result = true;
    if (token1->contextInfo == token2->contextInfo)
        return compareValues(token1->value, token2->value);

    return compareByContext(token1->contextInfo, token2->contextInfo, contextTables, true);
}

bool CompletionComparer::compareColumnsForCreateTable(const ExpectedTokenPtr& token1, const ExpectedTokenPtr& token2, bool* result)
{
    *result = true;

    bool token1OnAvailableList = helper->favoredColumnNames.contains(token1->value) && contextTables.contains(token1->contextInfo);
    bool token2OnAvailableList = helper->favoredColumnNames.contains(token2->value) && contextTables.contains(token2->contextInfo);
    if (token1OnAvailableList && !token2OnAvailableList)
        return true;

    if (!token1OnAvailableList && token2OnAvailableList)
        return false;

    *result = false;
    return false;
}

bool CompletionComparer::compareColumnsForReturning(const ExpectedTokenPtr& token1, const ExpectedTokenPtr& token2, bool* result)
{
    *result = true;

    // Checking if columns are on list of columns available in FROM clause
    bool token1available = isTokenOnAvailableColumnList(token1);
    bool token2available = isTokenOnAvailableColumnList(token2);
    if (token1available && !token2available)
        return true;

    if (!token1available && token2available)
        return false;

    *result = false;
    return false;
}

bool CompletionComparer::compareTables(const ExpectedTokenPtr& token1, const ExpectedTokenPtr& token2)
{
    if (!helper->parsedQuery || helper->parsedQuery->queryType != SqliteQueryType::Select)
        return compareValues(token1, token2);

    if (helper->context == CompletionHelper::Context::SELECT_FROM)
    {
        // In case the table was already mentioned in any FROM clause, we push it back.
        bool token1OnAvailableList = availableTableNames.contains(token1->value);
        bool token2OnAvailableList = availableTableNames.contains(token2->value);
        if (token1OnAvailableList && !token2OnAvailableList)
            return false;

        if (!token1OnAvailableList && token2OnAvailableList)
            return true;
    }

    bool ok;
    bool result = compareByContext(token1->value, token2->value, contextTables, &ok);
    if (ok)
        return result;

    result = compareByContext(token1->contextInfo, token2->contextInfo, contextDatabases, &ok);
    if (ok)
        return result;

    result = compareByContext(token1->value, token2->value, parentContextTables, &ok);
    if (ok)
        return result;

    result = compareByContext(token1->contextInfo, token2->contextInfo, parentContextDatabases, &ok);
    if (ok)
        return result;

    return compareValues(token1->value, token2->value, true);
}

bool CompletionComparer::compareIndexes(const ExpectedTokenPtr& token1, const ExpectedTokenPtr& token2)
{
    return compareValues(token1, token2, true);
}

bool CompletionComparer::compareTriggers(const ExpectedTokenPtr& token1, const ExpectedTokenPtr& token2)
{
    return compareValues(token1, token2);
}

bool CompletionComparer::compareViews(const ExpectedTokenPtr& token1, const ExpectedTokenPtr& token2)
{
    return compareValues(token1, token2);
}

bool CompletionComparer::compareDatabases(const ExpectedTokenPtr& token1, const ExpectedTokenPtr& token2)
{
    if (!helper->parsedQuery || helper->parsedQuery->queryType != SqliteQueryType::Select)
        return compareValues(token1, token2);

    return compareByContext(token1->value, token2->value, {contextDatabases, parentContextDatabases});
}

bool CompletionComparer::compareValues(const ExpectedTokenPtr &token1, const ExpectedTokenPtr &token2, bool handleSystemNames)
{
    return compareValues(token1->value, token2->value, handleSystemNames);
}

bool CompletionComparer::compareValues(const QString &token1, const QString &token2, bool handleSystemNames)
{
    // qDebug() << "comparing" << token1 << "and" << token2 << "=" << token1.compare(token2, Qt::CaseInsensitive);
    if (handleSystemNames)
    {
        bool firstIsSystem = token1.toLower().startsWith("sqlite_");
        bool secondIsSystem = token2.toLower().startsWith("sqlite_");
        if (firstIsSystem && !secondIsSystem)
            return false;

        if (!firstIsSystem && secondIsSystem)
            return true;
    }

    return token1.compare(token2, Qt::CaseInsensitive) < 0;
}

bool CompletionComparer::compareByContext(const QString &token1, const QString &token2, const QStringList &contextValues, bool *ok)
{
    return compareByContext(token1, token2, contextValues, false, ok);
}

bool CompletionComparer::compareByContext(const QString &token1, const QString &token2, const QList<QStringList> &contextValues, bool *ok)
{
    return compareByContext(token1, token2, contextValues, false, ok);
}

bool CompletionComparer::compareByContext(const QString &token1, const QString &token2, const QStringList &contextValues, bool handleSystemNames, bool* ok)
{
    if (ok)
        *ok = true;

    bool localOk = false;
    bool result = compareByContextOnly(token1, token2, contextValues, handleSystemNames, &localOk);

    if (localOk)
        return result;

    // Otherwise we compare by value.
    if (ok)
        *ok = false;

    return compareValues(token1, token2, handleSystemNames);
}

bool CompletionComparer::compareByContext(const QString &token1, const QString &token2, const QList<QStringList>& contextValues, bool handleSystemNames, bool* ok)
{
    if (ok)
        *ok = true;

    bool localOk = false;
    bool result;

    for (const QStringList& ctxValues : contextValues)
    {
        result = compareByContextOnly(token1, token2, ctxValues, handleSystemNames, &localOk);
        if (localOk)
            return result;
    }

    // Otherwise we compare by value.
    if (ok)
        *ok = false;

    return compareValues(token1, token2, handleSystemNames);
}

bool CompletionComparer::compareByContextOnly(const QString &token1, const QString &token2, const QStringList &contextValues, bool handleSystemNames, bool *ok)
{
    *ok = true;

    bool token1InContext = contextValues.contains(token1);
    bool token2InContext = contextValues.contains(token2);

    // token1 < token2 is true only if token1 is in context and token2 is not.
    // This means that token1 will be on the list before token2.
    if (token1InContext && !token2InContext)
        return true;

    // If token2 is in context, but token1 is not, then it's definite false.
    if (!token1InContext && token2InContext)
        return false;

    if (handleSystemNames)
    {
        bool firstIsSystem = token1.toLower().startsWith("sqlite_");
        bool secondIsSystem = token2.toLower().startsWith("sqlite_");
        if (firstIsSystem && !secondIsSystem)
            return false;

        if (!firstIsSystem && secondIsSystem)
            return true;
    }

    *ok = false;
    return false;
}

bool CompletionComparer::isTokenOnAvailableColumnList(const ExpectedTokenPtr &token)
{
    if (helper->parsedQuery->queryType == SqliteQueryType::Select)
        return isTokenOnColumnList(token, helper->selectAvailableColumns);
    else
        return isTokenOnColumnList(token, helper->theFromAvailableColumns);
}

bool CompletionComparer::isTokenOnParentAvailableColumnList(const ExpectedTokenPtr &token)
{
    return isTokenOnColumnList(token, helper->parentSelectAvailableColumns);
}

bool CompletionComparer::isTokenOnResultColumns(const ExpectedTokenPtr &token)
{
    return isTokenOnColumnList(token, resultColumns);
}

bool CompletionComparer::isTokenOnColumnList(const ExpectedTokenPtr &token, const QList<SelectResolver::Column> &columnList)
{
    for (SelectResolver::Column column : columnList)
    {
        // If column name doesn't match, then it's not this column
        if (token->value.compare(column.column, Qt::CaseInsensitive) != 0)
            continue;

        // At this point, column name is matched
        if (token->prefix.isNull() && token->contextInfo.isNull())
        {
            // No prefix, nor context info, just column name.
            return true;
        }

        // Table alias or just table name?
        QString toCompareWithPrefix;
        if (!column.tableAlias.isNull())
            toCompareWithPrefix = column.tableAlias;
        else
            toCompareWithPrefix = column.table;

        // Do we have actual prefix, or just context information and transparent (null) prefix?
        QString prefix;
//        if (!token->prefix.isNull())
//            prefix = token->prefix;
//        else
            prefix = token->contextInfo;

        // Does the table/alias match prefix?
        if (prefix.compare(column.table, Qt::CaseInsensitive) == 0)
            return true;
    }
    return false;
}

