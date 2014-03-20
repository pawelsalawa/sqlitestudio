#include "selectresolver.h"
#include "parser/token.h"
#include "parser/lexer.h"
#include "parser/keywords.h"
#include "schemaresolver.h"
#include "parser/ast/sqlitecreateview.h"

#include <QDebug>
#include <QHash>
#include <QHashIterator>
#include <QString>

SelectResolver::SelectResolver(Db *db, const QString& originalQuery)
{
    this->db = db;
    this->query = originalQuery;
    schemaResolver = new SchemaResolver(db);
}

SelectResolver::SelectResolver(Db* db, const QString& originalQuery, const BiStrHash& dbNameToAttach) :
    SelectResolver(db, originalQuery)
{
    this->dbNameToAttach = dbNameToAttach;
}

QList<SelectResolver::Column> SelectResolver::resolve(SqliteSelect::Core *selectCore)
{
    if (selectCore->from)
        currentCoreSourceColumns = resolveJoinSource(selectCore->from);

    foreach (SqliteSelect::Core::ResultColumn* resCol, selectCore->resultColumns)
        resolve(resCol);

    if (selectCore->distinctKw)
        markDistinctColumns();

    if (selectCore->groupBy.size() > 0)
        markGroupedColumns();

    fixColumnNames();

    SqliteSelect* select = dynamic_cast<SqliteSelect*>(selectCore->parentStatement());
    if (select && select->coreSelects.size() > 1)
        markCompoundColumns();

    if (select && select->with)
        markCteColumns();

    return currentCoreResults;
}

QList<QList<SelectResolver::Column> > SelectResolver::resolve(SqliteSelect *select)
{
    QList<QList<SelectResolver::Column> > results;
    foreach (SqliteSelect::Core* core, select->coreSelects)
    {
        results << resolve(core);
        currentCoreResults.clear();
    }

    return results;
}

QList<SelectResolver::Column> SelectResolver::resolveAvailableColumns(SqliteSelect::Core *selectCore)
{
    QList<Column> columns;
    if (selectCore->from)
        columns = resolveJoinSource(selectCore->from);

    SqliteSelect* select = dynamic_cast<SqliteSelect*>(selectCore->parentStatement());
    if (select && select->with)
        markCteColumns();

    return columns;
}

QList<QList<SelectResolver::Column> > SelectResolver::resolveAvailableColumns(SqliteSelect *select)
{
    QList<QList<SelectResolver::Column> > results;
    foreach (SqliteSelect::Core* core, select->coreSelects)
        results << resolveAvailableColumns(core);

    return results;
}

QSet<SelectResolver::Table> SelectResolver::resolveTables(SqliteSelect::Core *selectCore)
{
    QSet<Table> tables;
    QList<Column> columns = resolveAvailableColumns(selectCore);
    foreach (Column col, columns)
        tables << col.getTable();

    return tables;
}

QList<QSet<SelectResolver::Table> > SelectResolver::resolveTables(SqliteSelect *select)
{
    QList<QSet<Table> > results;
    QList<QList<Column> > columnLists = resolveAvailableColumns(select);
    foreach (QList<Column> columns, columnLists)
    {
        QSet<Table> tables;
        foreach (Column col, columns)
            tables << col.getTable();

        results << tables;
    }

    return results;
}

QList<SelectResolver::Column> SelectResolver::translateToColumns(SqliteSelect* select, const TokenList& columnTokens)
{
    QList<SelectResolver::Column> results;
    foreach (TokenPtr token, columnTokens)
        results << translateToColumns(select, token);

    return results;
}

SelectResolver::Column SelectResolver::translateToColumns(SqliteSelect* select, TokenPtr token)
{
    // Default result
    Column notTranslatedColumn;
    notTranslatedColumn.type = Column::OTHER;
    notTranslatedColumn.column = token->value;

    // Find containing statement
    SqliteStatement* parentStmt = select->findStatementWithToken(token);
    if (!parentStmt)
    {
        qDebug() << "Could not find containing statement for given token while translating column token:" << token->toString()
                 << "Select tokens:" << select->tokens.toString();

        return notTranslatedColumn;
    }

    // Go through all select cores, from the most deep, to the most shallow
    SqliteSelect::Core* core = nullptr;
    while (parentStmt)
    {
        // Find nearest SELECT core.
        while (parentStmt && !(core = dynamic_cast<SqliteSelect::Core*>(parentStmt)))
            parentStmt = parentStmt->parentStatement();

        if (!core)
        {
            qDebug() << "Could not find SqliteSelect::Core object for given token while translating column token:" << token->toString()
                     << "Select:" << select->detokenize();

            return notTranslatedColumn;
        }

        // Search through available columns
        foreach (const Column& availableColumn, resolveAvailableColumns(core))
        {
            if (availableColumn.type == Column::COLUMN && availableColumn.column.compare(token->value, Qt::CaseInsensitive) == 0)
                return availableColumn;
        }

        // Not in this core. See if there is any core upper (if this was a subselect).
        parentStmt = parentStmt->parentStatement();
    }

    return notTranslatedColumn;
}

void SelectResolver::markDistinctColumns()
{
    markCurrentColumnsWithFlag(FROM_DISTINCT_SELECT);
}

void SelectResolver::markCompoundColumns()
{
    markCurrentColumnsWithFlag(FROM_COMPOUND_SELECT);
}

void SelectResolver::markCteColumns()
{
    markCurrentColumnsWithFlag(FROM_CTE_SELECT);
}

void SelectResolver::markGroupedColumns()
{
    markCurrentColumnsWithFlag(FROM_GROUPED_SELECT);
}

void SelectResolver::fixColumnNames()
{
    QSet<QString> existingDisplayNames;
    QString originalName;
    int i;

    QMutableListIterator<Column> it(currentCoreResults);
    while (it.hasNext())
    {
        originalName = it.next().displayName;
        for (i = 1; existingDisplayNames.contains(it.value().displayName); i++)
            it.value().displayName = originalName + ":" + QString::number(i);

        existingDisplayNames << it.value().displayName;
    }
}

void SelectResolver::markCurrentColumnsWithFlag(SelectResolver::Flag flag)
{
    QMutableListIterator<Column> it(currentCoreResults);
    while (it.hasNext())
        it.next().flags |= flag;
}

void SelectResolver::resolve(SqliteSelect::Core::ResultColumn *resCol)
{
    if (resCol->star)
        resolveStar(resCol);
    else
        resolveExpr(resCol);
}

void SelectResolver::resolveStar(SqliteSelect::Core::ResultColumn *resCol)
{
    foreach (SelectResolver::Column column, currentCoreSourceColumns)
    {
        if (!resCol->table.isNull())
        {
            /*
             * Star was prefixed with table or table alias.
             * The "FROM" clause allows to use alias name the same as
             * some other table real name in the very same "FROM".
             * Their columns concatenate, so here we allow any column that
             * prefix matches either alias or table from data source list.
             * For example it's correct to query:
             *     SELECT test.* FROM test, otherTable AS test;
             * This case is simpler then in resolveDbAndTable(),
             * because here's no database allowed.
             *
             * Also, if the table has an alias specified,
             * then the alias has a precedence before table's name,
             * therefore we match table name only if the table alias
             * is null.
             */
            if (
                    (
                        !column.tableAlias.isNull() &&
                        resCol->table.compare(column.tableAlias, Qt::CaseInsensitive) != 0
                    ) ||
                    (
                        column.tableAlias.isNull() &&
                        resCol->table.compare(column.table, Qt::CaseInsensitive) != 0
                    )

                )
            {
                continue;
            }

            // If source column name is aliased, use it
            if (!column.alias.isNull())
                column.displayName = column.alias;
            else
                column.displayName = (column.tableAlias.isNull() ? column.table : column.tableAlias) +
                    "." + column.column;
        }
        else
        {
            // If source column name is aliased, use it
            if (!column.alias.isNull())
                column.displayName = column.alias;
            else
                column.displayName = column.column;
        }

        column.originalColumn = resCol;
        currentCoreResults << column;
    }
}

void SelectResolver::resolveExpr(SqliteSelect::Core::ResultColumn *resCol)
{
    SelectResolver::Column column;
    column.alias = resCol->alias;
    column.originalColumn = resCol;
    column.column = getResColTokensWithoutAlias(resCol).detokenize().trimmed();
    column.displayName = !resCol->alias.isNull() ? column.alias : column.column;

    SqliteExpr* expr = resCol->expr;
    if (expr->mode != SqliteExpr::Mode::ID)
    {
        // Not a simple column, but some expression
        column.type = Column::OTHER;
        currentCoreResults << column;

        // In this case we end it here.
        return;
    }

    // Now we know we're dealing with db.table.column (with db and table optional)
    resolveDbAndTable(resCol);
}

void SelectResolver::resolveDbAndTable(SqliteSelect::Core::ResultColumn *resCol)
{
    SqliteExpr* expr = resCol->expr;

    // Basic info
    Column col;
    col.alias = resCol->alias;
    col.column = expr->column;
    col.originalColumn = resCol;
    col.type = Column::COLUMN;

    // Display name
    if (col.alias.isNull())
    {
        QStringList dispNameList;
        if (!expr->database.isNull())
            dispNameList << resolveDatabase(expr->database);

        if (!expr->table.isNull())
            dispNameList << expr->table;

        dispNameList << expr->column;
        col.displayName = dispNameList.join(".");
    }
    else
        col.displayName = col.alias;

    // Looking for table relation
    Column matched;
    if (isRowIdKeyword(expr->column))
        matched = resolveRowIdColumn(expr);
    else if (!expr->database.isNull())
        matched = resolveExplicitColumn(expr->database, expr->table, expr->column);
    else if (!expr->table.isNull())
        matched = resolveExplicitColumn(expr->table, expr->column);
    else
        matched = resolveExplicitColumn(expr->column);


    if (!matched.table.isNull())
    {
        col.database = matched.database;
        col.originalDatabase = resolveDatabase(matched.database);
        col.table = matched.table;
        col.tableAlias = matched.tableAlias;
        col.flags = matched.flags;
    }
    else if (!ignoreInvalidNames)
    {
        qDebug() << "Source table for column '" << expr->detokenize()
                 << "' not matched while resolving select: " << query;
    }

    currentCoreResults << col;
}

SelectResolver::Column SelectResolver::resolveRowIdColumn(SqliteExpr *expr)
{
    // Looking for first source that can provide ROWID.
    foreach (Column column, currentCoreSourceColumns)
    {
        if (column.table.isNull())
            continue; // ROWID cannot be related to source with no table

        if (!expr->table.isNull() && matchTable(column, expr->table))
            return column;
    }
    return Column();
}

SelectResolver::Column SelectResolver::resolveExplicitColumn(const QString &columnName)
{
    foreach (const Column& column, currentCoreSourceColumns)
    {
        if (columnName.compare(column.column, Qt::CaseInsensitive) != 0 && columnName.compare(column.alias, Qt::CaseInsensitive) != 0)
            continue;

        return column;
    }
    return Column();
}

SelectResolver::Column SelectResolver::resolveExplicitColumn(const QString &table, const QString &columnName)
{
    foreach (const Column& column, currentCoreSourceColumns)
    {
        if (columnName.compare(column.column, Qt::CaseInsensitive) != 0 && columnName.compare(column.alias, Qt::CaseInsensitive) != 0)
            continue;

        if (!matchTable(column, table))
            continue;

        return column;
    }
    return Column();
}

SelectResolver::Column SelectResolver::resolveExplicitColumn(const QString &database, const QString &table, const QString &columnName)
{
    foreach (const Column& column, currentCoreSourceColumns)
    {
        if (columnName.compare(column.column, Qt::CaseInsensitive) != 0 && columnName.compare(column.alias, Qt::CaseInsensitive) != 0)
            continue;

        if (!matchTable(column, table))
            continue;

        if (database.compare(column.database, Qt::CaseInsensitive) != 0)
            continue;

        return column;
    }
    return Column();
}

bool SelectResolver::matchTable(const SelectResolver::Column &sourceColumn, const QString &table)
{
    // First check by tableAlias if it's present
    if (!sourceColumn.tableAlias.isNull())
        return (sourceColumn.tableAlias.compare(table, Qt::CaseInsensitive) == 0);

    return (sourceColumn.table.compare(table, Qt::CaseInsensitive) == 0);
}

TokenList SelectResolver::getResColTokensWithoutAlias(SqliteSelect::Core::ResultColumn *resCol)
{
    TokenList allTokens = resCol->tokens;
    if (!resCol->alias.isNull())
    {
        int idx = allTokens.indexOf(Token::KEYWORD, "AS", Qt::CaseInsensitive);
        if (idx > -1)
            allTokens = allTokens.mid(0, idx - 1);
    }

    return allTokens;
}

QList<SelectResolver::Column> SelectResolver::resolveJoinSource(SqliteSelect::Core::JoinSource *joinSrc)
{
    QList<SelectResolver::Column> columnSources;
    columnSources += resolveSingleSource(joinSrc->singleSource);
    foreach (SqliteSelect::Core::JoinSourceOther* otherSrc, joinSrc->otherSources)
        columnSources += resolveOtherSource(otherSrc);

    return columnSources;
}

QList<SelectResolver::Column> SelectResolver::resolveSingleSource(SqliteSelect::Core::SingleSource *joinSrc)
{
    if (!joinSrc)
        return QList<Column>();

    if (joinSrc->select)
        return resolveSingleSourceSubSelect(joinSrc);

    if (joinSrc->joinSource)
        return resolveJoinSource(joinSrc->joinSource);

    if (isView(joinSrc->database, joinSrc->table))
        return resolveView(joinSrc->database, joinSrc->table, joinSrc->alias);

    QList<Column> columnSources;
    QStringList columns = getTableColumns(joinSrc->database, joinSrc->table, joinSrc->alias);
    Column column;
    foreach (QString columnName, columns)
    {
        column.type = Column::COLUMN;
        column.column = columnName;
        column.table = joinSrc->table;;
        column.database = joinSrc->database;
        column.originalDatabase = resolveDatabase(joinSrc->database);
        if (!joinSrc->alias.isNull())
            column.tableAlias = joinSrc->alias;

        columnSources << column;
    }

    return columnSources;
}

QList<SelectResolver::Column> SelectResolver::resolveSingleSourceSubSelect(SqliteSelect::Core::SingleSource *joinSrc)
{
    QList<Column> columnSources = resolveSubSelect(joinSrc->select);
    applySubSelectAlias(columnSources, joinSrc->alias);
    return columnSources;
}

QList<SelectResolver::Column> SelectResolver::resolveOtherSource(SqliteSelect::Core::JoinSourceOther *otherSrc)
{
    return resolveSingleSource(otherSrc->singleSource);
}

QList<SelectResolver::Column> SelectResolver::resolveSubSelect(SqliteSelect *select)
{
    QList<Column> columnSources;
    Q_ASSERT(select->coreSelects.size() > 0);

    bool compound = (select->coreSelects.size() > 1);

    if (compound && !resolveMultiCore)
        return columnSources;

    SelectResolver internalResolver(db, query);
    columnSources += internalResolver.resolve(select->coreSelects[0]);

    if (compound)
    {
        QMutableListIterator<Column> it(columnSources);
        while (it.hasNext())
            it.next().flags |= FROM_COMPOUND_SELECT;
    }

    return columnSources;
}

QList<SelectResolver::Column> SelectResolver::resolveView(const QString& database, const QString& name, const QString& alias)
{
    QList<Column> results;
    SqliteQueryPtr query = schemaResolver->getParsedObject(database, name);
    if (!query)
    {
        qDebug() << "Could not get parsed CREATE VIEW in SelectResolver::resolveView().";
        return results;
    }

    SqliteCreateViewPtr createView = query.dynamicCast<SqliteCreateView>();
    if (!createView)
    {
        qDebug() << "Parsed object not a CREATE VIEW as expected, but instead it's:" << sqliteQueryTypeToString(query->queryType);
        return results;
    }

    results = resolveSubSelect(createView->select);
    applySubSelectAlias(results, (!alias.isNull() ? alias : name));

    return results;
}

bool SelectResolver::isView(const QString& database, const QString& name)
{
    return schemaResolver->getViews(database).contains(name, Qt::CaseInsensitive);
}

QStringList SelectResolver::getTableColumns(const QString &database, const QString &table, const QString& alias)
{
    Table dbTable;
    dbTable.database = database;
    dbTable.table = table;
    dbTable.alias = alias;

    if (tableColumnsCache.contains(dbTable))
        return tableColumnsCache.value(dbTable);
    else
    {
        QStringList columns = schemaResolver->getTableColumns(database, table);
        tableColumnsCache[dbTable] = columns;
        return columns;
    }
}

void SelectResolver::applySubSelectAlias(QList<SelectResolver::Column>& columns, const QString& alias)
{
    // If this subselect is aliased, then all source columns should be considered as from aliased table
    QMutableListIterator<Column> it(columns);
    if (!alias.isNull())
    {
        while (it.hasNext())
        {
            it.next().tableAlias = alias;
            it.value().flags &= ~FROM_ANONYMOUS_SELECT; // remove anonymous flag
        }
    }
    else
    {
        // Otherwise, mark column as being from anonymous subselect.
        // This is used by QueryExecutorColumns step to avoid prefixing result column with table
        // when it comes from anonymous subselect (which SQLite needs this to be not prefixed column).
        while (it.hasNext())
            it.next().flags |= FROM_ANONYMOUS_SELECT;
    }
}

QString SelectResolver::resolveDatabase(const QString& database)
{
    if (dbNameToAttach.containsRight(database, Qt::CaseInsensitive))
        return dbNameToAttach.valueByRight(database, Qt::CaseInsensitive);

    return database;
}

int SelectResolver::Table::operator ==(const SelectResolver::Table &other)
{
    return table == other.table && database == other.database && alias == other.alias;
}

int operator==(const SelectResolver::Table& t1, const SelectResolver::Table& t2)
{
    return t1.table == t2.table && t1.database == t2.database && t1.alias == t2.alias;
}

uint qHash(const SelectResolver::Table& table)
{
    return qHash(table.database + "." + table.table + "." + table.alias);
}

int SelectResolver::Column::operator ==(const SelectResolver::Column &other)
{
    return table == other.table && database == other.database && column == other.column && tableAlias == other.tableAlias;
}

SelectResolver::Table SelectResolver::Column::getTable()
{
    Table resTable;
    resTable.table = table;
    resTable.database = database;
    resTable.originalDatabase = originalDatabase;
    resTable.alias = tableAlias;
    resTable.flags = flags;
    return resTable;
}

int operator ==(const SelectResolver::Column &c1, const SelectResolver::Column &c2)
{
    return c1.table == c2.table && c1.database == c2.database && c1.column == c2.column && c1.tableAlias == c2.tableAlias;
}


uint qHash(const SelectResolver::Column &column)
{
    return qHash(column.database + "." + column.table + "." + column.column + "/" + column.tableAlias);
}
