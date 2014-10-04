#include "formatcolumntype.h"
#include "formatcreatetable.h"
#include "sqlenterpriseformatter.h"

FormatCreateTable::FormatCreateTable(SqliteCreateTable* createTable) :
    createTable(createTable)
{
}

void FormatCreateTable::formatInternal()
{
    withKeyword("CREATE");
    if (createTable->tempKw)
        withKeyword("TEMP");
    else if (createTable->temporaryKw)
        withKeyword("TEMPORARY");

    withKeyword("TABLE");
    if (createTable->ifNotExistsKw)
        withKeyword("IF").withKeyword("NOT").withKeyword("EXISTS");

    if (dialect == Dialect::Sqlite3 && !createTable->database.isNull())
        withId(createTable->database).withIdDot();

    withId(createTable->table);

    if (createTable->select)
        withKeyword("AS").withStatement(createTable->select);
    else
    {
        withParDefLeft();
        formatColumns(createTable->columns);
        if (createTable->constraints.size() > 0)
            withListComma().withStatementList(createTable->constraints);

        withParDefRight();

        if (!createTable->withOutRowId.isNull())
            withKeyword("WITHOUT").withId("ROWID");
    }

    withSemicolon();
}

void FormatCreateTable::formatColumns(const QList<SqliteCreateTable::Column*>& columns)
{
    int maxColNameIndent = 0;
    int maxColTypeIndent = 0;
    FormatColumnType* formatColType = nullptr;
    foreach (SqliteCreateTable::Column* stmt, columns)
    {
        maxColNameIndent = qMax(getColNameLength(stmt->name), maxColNameIndent);

        if (stmt->type)
        {

            formatColType = getFormatStatement<FormatColumnType>(stmt->type);
            maxColTypeIndent = qMax(formatColType->format().trimmed().length(), maxColTypeIndent);
            delete formatColType;
        }
    }

    if (columns.size() > 1)
    {
        maxColNameIndent++; // for a single whitespace to line up with other columns
        maxColTypeIndent++; // the same for constraints
    }

    withStatementList(columns, "columns", ListSeparator::COMMA, [maxColNameIndent, maxColTypeIndent](FormatStatement* formatStmt)
    {
        FormatCreateTableColumn* colStmt = dynamic_cast<FormatCreateTableColumn*>(formatStmt);
        if (colStmt)
        {
            colStmt->setColNameIndent(maxColNameIndent);
            colStmt->setColTypeIndent(maxColTypeIndent);
        }
    });
}

int FormatCreateTable::getColNameLength(const QString& name)
{
    if (cfg->SqlEnterpriseFormatter.AlwaysUseNameWrapping.get())
        return wrapObjName(name, dialect, wrapper).length();
    else
        return wrapObjIfNeeded(name, dialect, wrapper).length();
}

FormatCreateTableColumn::FormatCreateTableColumn(SqliteCreateTable::Column* column) :
    column(column)
{
}

void FormatCreateTableColumn::setColNameIndent(int value)
{
    colNameIndent = value;
}

void FormatCreateTableColumn::setColTypeIndent(int value)
{
    colTypeIndent = value;
}

void FormatCreateTableColumn::formatInternal()
{
    ListSeparator sep = ListSeparator::NONE;
    if (cfg->SqlEnterpriseFormatter.NlBetweenConstraints.get())
        sep = ListSeparator::NEW_LINE;

    withId(column->name).withIncrIndent(colNameIndent).withStatement(column->type).withIncrIndent(colTypeIndent)
            .withStatementList(column->constraints, QString(), sep).withDecrIndent().withDecrIndent();
}


FormatCreateTableColumnConstraint::FormatCreateTableColumnConstraint(SqliteCreateTable::Column::Constraint* constr) :
    constr(constr)
{
}

void FormatCreateTableColumnConstraint::formatInternal()
{
    if (!constr->name.isNull())
        withKeyword("CONSTRAINT").withId(constr->name);

    switch (constr->type)
    {
        case SqliteCreateTable::Column::Constraint::PRIMARY_KEY:
        {
            withKeyword("PRIMARY").withKeyword("KEY").withSortOrder(constr->sortOrder).withConflict(constr->onConflict);
            if (constr->autoincrKw)
                withKeyword("AUTOINCREMENT");

            break;
        }
        case SqliteCreateTable::Column::Constraint::NOT_NULL:
        {
            withKeyword("NOT").withKeyword("NULL").withConflict(constr->onConflict);
            break;
        }
        case SqliteCreateTable::Column::Constraint::UNIQUE:
        {
            withKeyword("UNIQUE").withConflict(constr->onConflict);
            break;
        }
        case SqliteCreateTable::Column::Constraint::CHECK:
        {
            withKeyword("CHECK").withParExprLeft().withStatement(constr->expr).withParExprRight().withConflict(constr->onConflict);
            break;
        }
        case SqliteCreateTable::Column::Constraint::DEFAULT:
        {
            withKeyword("DEFAULT");
            if (!constr->id.isNull())
                withId(constr->id);
            else if (!constr->ctime.isNull())
                withKeyword(constr->ctime);
            else if (constr->expr)
                withParExprLeft().withStatement(constr->expr).withParExprRight();
            else if (constr->literalNull)
                withKeyword("NULL");
            else
                withLiteral(constr->literalValue);

            break;
        }
        case SqliteCreateTable::Column::Constraint::COLLATE:
        {
            withKeyword("COLLATE").withId(constr->collationName);
            break;
        }
        case SqliteCreateTable::Column::Constraint::FOREIGN_KEY:
        {
            withStatement(constr->foreignKey);
            break;
        }
        case SqliteCreateTable::Column::Constraint::NULL_:
        case SqliteCreateTable::Column::Constraint::NAME_ONLY:
        case SqliteCreateTable::Column::Constraint::DEFERRABLE_ONLY:
            break;
    }
}


FormatCreateTableConstraint::FormatCreateTableConstraint(SqliteCreateTable::Constraint* constr) :
    constr(constr)
{
}

void FormatCreateTableConstraint::formatInternal()
{
    if (!constr->name.isNull())
        withKeyword("CONSTRAINT").withId(constr->name);

    switch (constr->type)
    {
        case SqliteCreateTable::Constraint::PRIMARY_KEY:
        {
            withKeyword("PRIMARY").withKeyword("KEY").withParDefLeft().withStatementList(constr->indexedColumns).withParDefRight();

            if (constr->autoincrKw)
                withKeyword("AUTOINCREMENT");

            withConflict(constr->onConflict);
            break;
        }
        case SqliteCreateTable::Constraint::UNIQUE:
        {
            withKeyword("UNIQUE").withParDefLeft().withStatementList(constr->indexedColumns).withParDefRight().withConflict(constr->onConflict);
            break;
        }
        case SqliteCreateTable::Constraint::CHECK:
        {
            withKeyword("CHECK").withParExprLeft().withStatement(constr->expr).withParExprRight().withConflict(constr->onConflict);
            break;
        }
        case SqliteCreateTable::Constraint::FOREIGN_KEY:
        {
            withKeyword("FOREIGN").withKeyword("KEY").withParDefLeft().withStatementList(constr->indexedColumns)
                    .withParDefRight().withStatement(constr->foreignKey);
            break;
        }
        case SqliteCreateTable::Constraint::NAME_ONLY:
            break;
    }
}
