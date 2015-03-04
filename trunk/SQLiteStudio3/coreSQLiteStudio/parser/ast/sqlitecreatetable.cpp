#include "sqlitecreatetable.h"
#include "parser/statementtokenbuilder.h"
#include "common/utils_sql.h"
#include "common/global.h"

SqliteCreateTable::SqliteCreateTable()
{
    queryType = SqliteQueryType::CreateTable;
}

SqliteCreateTable::SqliteCreateTable(const SqliteCreateTable& other) :
    SqliteQuery(other), ifNotExistsKw(other.ifNotExistsKw), tempKw(other.tempKw), temporaryKw(other.temporaryKw),
    database(other.database), table(other.table), withOutRowId(other.withOutRowId)
{
    DEEP_COPY_COLLECTION(Column, columns);
    DEEP_COPY_COLLECTION(Constraint, constraints);
    DEEP_COPY_FIELD(SqliteSelect, select);
}

SqliteCreateTable::SqliteCreateTable(bool ifNotExistsKw, int temp, const QString &name1, const QString &name2, const QList<Column *> &columns, const QList<Constraint*>& constraints)
    : SqliteCreateTable()
{
    init(ifNotExistsKw, temp, name1, name2);
    this->columns = columns;
    foreach (Column* column, columns)
        column->setParent(this);

    SqliteCreateTable::Constraint* constr = nullptr;
    foreach (constr, constraints)
    {
        if (this->constraints.size() > 0 &&
            this->constraints.last()->type == SqliteCreateTable::Constraint::NAME_ONLY)
        {
            constr->name = this->constraints.last()->name;
            delete this->constraints.takeLast();
        }
        this->constraints << constr;
        constr->setParent(this);
    }
}

SqliteCreateTable::SqliteCreateTable(bool ifNotExistsKw, int temp, const QString& name1, const QString& name2, const QList<SqliteCreateTable::Column*>& columns, const QList<SqliteCreateTable::Constraint*>& constraints, const QString& withOutRowId) :
    SqliteCreateTable(ifNotExistsKw, temp, name1, name2, columns, constraints)
{
    this->withOutRowId = withOutRowId;
}

SqliteCreateTable::SqliteCreateTable(bool ifNotExistsKw, int temp, const QString &name1, const QString &name2, SqliteSelect *select)
    : SqliteCreateTable()
{
    init(ifNotExistsKw, temp, name1, name2);
    this->select = select;
    if (select)
        select->setParent(this);
}

SqliteCreateTable::~SqliteCreateTable()
{
}

SqliteStatement*SqliteCreateTable::clone()
{
    return new SqliteCreateTable(*this);
}

QList<SqliteCreateTable::Constraint*> SqliteCreateTable::getConstraints(SqliteCreateTable::Constraint::Type type) const
{
    QList<SqliteCreateTable::Constraint*> results;
    foreach (Constraint* constr, constraints)
        if (constr->type == type)
            results << constr;

    return results;
}

SqliteStatement* SqliteCreateTable::getPrimaryKey() const
{
    foreach (Constraint* constr, getConstraints(Constraint::PRIMARY_KEY))
        return constr;

    Column::Constraint* colConstr = nullptr;
    foreach (Column* col, columns)
    {
        colConstr = col->getConstraint(Column::Constraint::PRIMARY_KEY);
        if (colConstr)
            return colConstr;
    }

    return nullptr;
}

QStringList SqliteCreateTable::getPrimaryKeyColumns() const
{
    QStringList colNames;
    SqliteStatement* primaryKey = getPrimaryKey();
    if (!primaryKey)
        return colNames;

    SqliteCreateTable::Column::Constraint* columnConstr = dynamic_cast<SqliteCreateTable::Column::Constraint*>(primaryKey);
    if (columnConstr)
    {
        colNames << dynamic_cast<SqliteCreateTable::Column*>(columnConstr->parentStatement())->name;
        return colNames;
    }

    SqliteCreateTable::Constraint* tableConstr = dynamic_cast<SqliteCreateTable::Constraint*>(primaryKey);
    if (tableConstr)
    {
        foreach (SqliteIndexedColumn* idxCol, tableConstr->indexedColumns)
            colNames << idxCol->name;
    }
    return colNames;
}

SqliteCreateTable::Column* SqliteCreateTable::getColumn(const QString& colName)
{
    foreach (Column* col, columns)
    {
        if (col->name.compare(colName, Qt::CaseInsensitive) == 0)
            return col;
    }
    return nullptr;
}

QList<SqliteCreateTable::Constraint*> SqliteCreateTable::getForeignKeysByTable(const QString& foreignTable) const
{
    QList<Constraint*> results;
    foreach (Constraint* constr, constraints)
        if (constr->type == Constraint::FOREIGN_KEY && constr->foreignKey->foreignTable.compare(foreignTable, Qt::CaseInsensitive) == 0)
            results << constr;

    return results;
}

QList<SqliteCreateTable::Column::Constraint*> SqliteCreateTable::getColumnForeignKeysByTable(const QString& foreignTable) const
{
    QList<Column::Constraint*> results;
    foreach (Column* col, columns)
        results += col->getForeignKeysByTable(foreignTable);

    return results;
}

QStringList SqliteCreateTable::getColumnNames() const
{
    QStringList names;
    foreach (Column* col, columns)
        names << col->name;

    return names;
}

QHash<QString, QString> SqliteCreateTable::getModifiedColumnsMap(bool lowercaseKeys, Qt::CaseSensitivity cs) const
{
    QHash<QString, QString> colMap;
    QString key;
    foreach (Column* col, columns)
    {
        key = lowercaseKeys ? col->originalName.toLower() : col->originalName;
        if (col->name.compare(col->originalName, cs) != 0)
            colMap[key] = col->name;
    }

    return colMap;
}

QStringList SqliteCreateTable::getTablesInStatement()
{
    return getStrListFromValue(table);
}

QStringList SqliteCreateTable::getDatabasesInStatement()
{
    return getStrListFromValue(database);
}

TokenList SqliteCreateTable::getTableTokensInStatement()
{
    return getObjectTokenListFromFullname();
}

TokenList SqliteCreateTable::getDatabaseTokensInStatement()
{
    return getDbTokenListFromFullname();
}

QList<SqliteStatement::FullObject> SqliteCreateTable::getFullObjectsInStatement()
{
    QList<FullObject> result;

    // Table object
    FullObject fullObj = getFullObjectFromFullname(FullObject::TABLE);

    if (fullObj.isValid())
        result << fullObj;

    // Db object
    fullObj = getFirstDbFullObject();
    if (fullObj.isValid())
    {
        result << fullObj;
        dbTokenForFullObjects = fullObj.database;
    }

    return result;
}

TokenList SqliteCreateTable::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;
    builder.withTokens(SqliteQuery::rebuildTokensFromContents());
    builder.withKeyword("CREATE");
    if (tempKw)
        builder.withSpace().withKeyword("TEMP");
    else if (temporaryKw)
        builder.withSpace().withKeyword("TEMPORARY");

    builder.withSpace().withKeyword("TABLE");
    if (ifNotExistsKw)
        builder.withSpace().withKeyword("IF").withSpace().withKeyword("NOT").withSpace().withKeyword("EXISTS");

    builder.withSpace();
    if (dialect == Dialect::Sqlite3 && !database.isNull())
        builder.withOther(database, dialect).withOperator(".");

    builder.withOther(table, dialect);

    if (select)
        builder.withSpace().withKeyword("AS").withSpace().withStatement(select);
    else
    {
        builder.withSpace().withParLeft().withStatementList(columns);
        if (constraints.size() > 0)
            builder.withOperator(",").withStatementList(constraints);

        builder.withParRight();

        if (!withOutRowId.isNull())
            builder.withSpace().withKeyword("WITHOUT").withSpace().withOther("ROWID");
    }

    builder.withOperator(";");

    return builder.build();
}

void SqliteCreateTable::init(bool ifNotExistsKw, int temp, const QString &name1, const QString &name2)
{
    this->ifNotExistsKw = ifNotExistsKw;
    if (temp == 2)
        temporaryKw = true;
    else if (temp == 1)
        tempKw = true;

    if (name2.isNull())
        table = name1;
    else
    {
        database = name1;
        table = name2;
    }
}


SqliteCreateTable::Column::Constraint::Constraint()
{
}

SqliteCreateTable::Column::Constraint::Constraint(const SqliteCreateTable::Column::Constraint& other) :
    SqliteStatement(other), type(other.type), name(other.name), sortOrder(other.sortOrder), onConflict(other.onConflict),
    autoincrKw(other.autoincrKw), literalValue(other.literalValue), literalNull(other.literalNull), ctime(other.ctime), id(other.id),
    collationName(other.collationName), deferrable(other.deferrable), initially(other.initially)
{
    DEEP_COPY_FIELD(SqliteExpr, expr);
    DEEP_COPY_FIELD(SqliteForeignKey, foreignKey);
}

SqliteCreateTable::Column::Constraint::~Constraint()
{
}

SqliteStatement* SqliteCreateTable::Column::Constraint::clone()
{
    return new SqliteCreateTable::Column::Constraint(*this);
}

void SqliteCreateTable::Column::Constraint::initDefNameOnly(const QString &name)
{
    this->type = SqliteCreateTable::Column::Constraint::NAME_ONLY;
    this->name = name;
}

void SqliteCreateTable::Column::Constraint::initDefId(const QString &id)
{
    this->type = SqliteCreateTable::Column::Constraint::DEFAULT;
    this->id = id;
}

void SqliteCreateTable::Column::Constraint::initDefTerm(const QVariant &value, bool minus)
{
    this->type = SqliteCreateTable::Column::Constraint::DEFAULT;
    if (minus)
    {
        if (value.type() == QVariant::Double)
            literalValue = -(value.toDouble());
        else if (value.type() == QVariant::LongLong)
            literalValue = -(value.toLongLong());
    }
    else if (value.isNull())
    {
        literalValue = value;
        literalNull = true;
    }
    else
        literalValue = value;
}

void SqliteCreateTable::Column::Constraint::initDefCTime(const QString &name)
{
    this->type = SqliteCreateTable::Column::Constraint::DEFAULT;
    ctime = name;
}

void SqliteCreateTable::Column::Constraint::initDefExpr(SqliteExpr *expr)
{
    this->type = SqliteCreateTable::Column::Constraint::DEFAULT;
    this->expr = expr;
    if (expr)
        expr->setParent(this);
}

void SqliteCreateTable::Column::Constraint::initNull(SqliteConflictAlgo algo)
{
    this->type = SqliteCreateTable::Column::Constraint::NULL_;
    onConflict = algo;
}

void SqliteCreateTable::Column::Constraint::initNotNull(SqliteConflictAlgo algo)
{
    this->type = SqliteCreateTable::Column::Constraint::NOT_NULL;
    onConflict = algo;
}

void SqliteCreateTable::Column::Constraint::initPk(SqliteSortOrder order, SqliteConflictAlgo algo, bool autoincr)
{
    this->type = SqliteCreateTable::Column::Constraint::PRIMARY_KEY;
    sortOrder = order;
    onConflict = algo;
    autoincrKw = autoincr;
}

void SqliteCreateTable::Column::Constraint::initUnique(SqliteConflictAlgo algo)
{
    this->type = SqliteCreateTable::Column::Constraint::UNIQUE;
    onConflict = algo;
}

void SqliteCreateTable::Column::Constraint::initCheck()
{
    this->type = SqliteCreateTable::Column::Constraint::CHECK;
}

void SqliteCreateTable::Column::Constraint::initCheck(SqliteExpr *expr)
{
    this->type = SqliteCreateTable::Column::Constraint::CHECK;
    this->expr = expr;
    if (expr)
        expr->setParent(this);
}

void SqliteCreateTable::Column::Constraint::initCheck(SqliteExpr *expr, SqliteConflictAlgo algo)
{
    initCheck(expr);
    this->onConflict = algo;
}

void SqliteCreateTable::Column::Constraint::initFk(const QString& table, const QList<SqliteIndexedColumn*>& indexedColumns, const QList<SqliteForeignKey::Condition*>& conditions)
{
    this->type = SqliteCreateTable::Column::Constraint::FOREIGN_KEY;

    SqliteForeignKey* fk = new SqliteForeignKey();
    fk->foreignTable = table;
    fk->indexedColumns = indexedColumns;
    fk->conditions = conditions;
    foreignKey = fk;
    fk->setParent(this);

    foreach (SqliteIndexedColumn* idxCol, indexedColumns)
        idxCol->setParent(fk);

    foreach (SqliteForeignKey::Condition* cond, conditions)
        cond->setParent(fk);
}

void SqliteCreateTable::Column::Constraint::initDefer(SqliteInitially initially, SqliteDeferrable deferrable)
{
    this->type = SqliteCreateTable::Column::Constraint::DEFERRABLE_ONLY;
    this->deferrable = deferrable;
    this->initially = initially;
}

void SqliteCreateTable::Column::Constraint::initColl(const QString &name)
{
    this->type = SqliteCreateTable::Column::Constraint::COLLATE;
    this->collationName = name;
}

QString SqliteCreateTable::Column::Constraint::typeString() const
{
    switch (type)
    {
        case SqliteCreateTable::Column::Constraint::PRIMARY_KEY:
            return "PRIMARY KEY";
        case SqliteCreateTable::Column::Constraint::NOT_NULL:
            return "NOT NULL";
        case SqliteCreateTable::Column::Constraint::UNIQUE:
            return "UNIQUE";
        case SqliteCreateTable::Column::Constraint::CHECK:
            return "CHECK";
        case SqliteCreateTable::Column::Constraint::DEFAULT:
            return "DEFAULT";
        case SqliteCreateTable::Column::Constraint::COLLATE:
            return "COLLATE";
        case SqliteCreateTable::Column::Constraint::FOREIGN_KEY:
            return "FOREIGN KEY";
        case SqliteCreateTable::Column::Constraint::NULL_:
        case SqliteCreateTable::Column::Constraint::NAME_ONLY:
        case SqliteCreateTable::Column::Constraint::DEFERRABLE_ONLY:
            break;
    }
    return QString::null;
}

SqliteCreateTable::Constraint::Constraint()
{
}

SqliteCreateTable::Constraint::Constraint(const SqliteCreateTable::Constraint& other) :
    SqliteStatement(other),  type(other.type), name(other.name), autoincrKw(other.autoincrKw), onConflict(other.onConflict),
    afterComma(other.afterComma)
{
    DEEP_COPY_FIELD(SqliteForeignKey, foreignKey);
    DEEP_COPY_FIELD(SqliteExpr, expr);
    DEEP_COPY_COLLECTION(SqliteIndexedColumn, indexedColumns);
}

SqliteCreateTable::Constraint::~Constraint()
{
}

SqliteStatement*SqliteCreateTable::Constraint::clone()
{
    return new SqliteCreateTable::Constraint(*this);
}

void SqliteCreateTable::Constraint::initNameOnly(const QString &name)
{
    this->type = SqliteCreateTable::Constraint::NAME_ONLY;
    this->name = name;
}

void SqliteCreateTable::Constraint::initPk(const QList<SqliteIndexedColumn *> &indexedColumns, bool autoincr, SqliteConflictAlgo algo)
{
    this->type = SqliteCreateTable::Constraint::PRIMARY_KEY;
    this->indexedColumns = indexedColumns;
    autoincrKw = autoincr;
    onConflict = algo;

    foreach (SqliteIndexedColumn* idxCol, indexedColumns)
        idxCol->setParent(this);
}

void SqliteCreateTable::Constraint::initUnique(const QList<SqliteIndexedColumn *> &indexedColumns, SqliteConflictAlgo algo)
{
    this->type = SqliteCreateTable::Constraint::UNIQUE;
    this->indexedColumns = indexedColumns;
    onConflict = algo;

    foreach (SqliteIndexedColumn* idxCol, indexedColumns)
        idxCol->setParent(this);
}

void SqliteCreateTable::Constraint::initCheck(SqliteExpr *expr, SqliteConflictAlgo algo)
{
    this->type = SqliteCreateTable::Constraint::CHECK;
    this->expr = expr;
    onConflict = algo;
    if (expr)
        expr->setParent(this);
}

void SqliteCreateTable::Constraint::initCheck()
{
    this->type = SqliteCreateTable::Constraint::CHECK;
}

void SqliteCreateTable::Constraint::initFk(const QList<SqliteIndexedColumn *> &indexedColumns, const QString& table, const QList<SqliteIndexedColumn *> &fkColumns, const QList<SqliteForeignKey::Condition *> &conditions, SqliteInitially initially, SqliteDeferrable deferrable)
{
    this->type = SqliteCreateTable::Constraint::FOREIGN_KEY;
    this->indexedColumns = indexedColumns;

    foreach (SqliteIndexedColumn* idxCol, indexedColumns)
        idxCol->setParent(this);

    SqliteForeignKey* fk = new SqliteForeignKey();
    fk->foreignTable = table;
    fk->indexedColumns = fkColumns;
    fk->conditions = conditions;
    fk->deferrable = deferrable;
    fk->initially = initially;

    fk->setParent(this);

    foreach (SqliteIndexedColumn* idxCol, fkColumns)
        idxCol->setParent(fk);

    foreach (SqliteForeignKey::Condition* cond, conditions)
        cond->setParent(fk);

    this->foreignKey = fk;
}

bool SqliteCreateTable::Constraint::doesAffectColumn(const QString& columnName)
{
    return getAffectedColumnIdx(columnName) > -1;
}

int SqliteCreateTable::Constraint::getAffectedColumnIdx(const QString& columnName)
{
    int i = 0;
    foreach (SqliteIndexedColumn* idxCol, indexedColumns)
    {
        if (idxCol->name.compare(columnName, Qt::CaseInsensitive) == 0)
            return i;

        i++;
    }

    return -1;
}

QString SqliteCreateTable::Constraint::typeString() const
{
    switch (type)
    {
        case SqliteCreateTable::Constraint::PRIMARY_KEY:
            return "PRIMARY KEY";
        case SqliteCreateTable::Constraint::UNIQUE:
            return "UNIQUE";
        case SqliteCreateTable::Constraint::CHECK:
            return "CHECK";
        case SqliteCreateTable::Constraint::FOREIGN_KEY:
            return "FOREIGN KEY";
        case SqliteCreateTable::Constraint::NAME_ONLY:
            return QString::null;
    }
    return QString::null;
}

TokenList SqliteCreateTable::Constraint::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;

    if (!name.isNull())
        builder.withKeyword("CONSTRAINT").withSpace().withOther(name, dialect).withSpace();

    switch (type)
    {
        case SqliteCreateTable::Constraint::PRIMARY_KEY:
        {
            builder.withKeyword("PRIMARY").withSpace().withKeyword("KEY").withSpace().withParLeft().withStatementList(indexedColumns).withParRight();

            if (autoincrKw)
                builder.withSpace().withKeyword("AUTOINCREMENT");

            builder.withConflict(onConflict);
            break;
        }
        case SqliteCreateTable::Constraint::UNIQUE:
        {
            builder.withKeyword("UNIQUE").withSpace().withParLeft().withStatementList(indexedColumns).withParRight().withConflict(onConflict);
            break;
        }
        case SqliteCreateTable::Constraint::CHECK:
        {
            builder.withKeyword("CHECK").withSpace().withParLeft().withStatement(expr).withParRight().withConflict(onConflict);
            break;
        }
        case SqliteCreateTable::Constraint::FOREIGN_KEY:
        {
            builder.withKeyword("FOREIGN").withSpace().withKeyword("KEY").withSpace().withParLeft().withStatementList(indexedColumns)
                    .withParRight().withStatement(foreignKey);
            break;
        }
        case SqliteCreateTable::Constraint::NAME_ONLY:
            break;
    }

    return builder.build();
}

SqliteCreateTable::Column::Column()
{
}

SqliteCreateTable::Column::Column(const SqliteCreateTable::Column& other) :
    SqliteStatement(other), name(other.name), originalName(other.originalName)
{
    DEEP_COPY_FIELD(SqliteColumnType, type);
    DEEP_COPY_COLLECTION(Constraint, constraints);
}

SqliteCreateTable::Column::Column(const QString &name, SqliteColumnType *type, const QList<Constraint *> &constraints)
{
    this->name = name;
    this->originalName = name;
    this->type = type;

    if (type)
        type->setParent(this);

    SqliteCreateTable::Column::Constraint* constr = nullptr;
    foreach (constr, constraints)
    {
        // If last constraint on list is NAME_ONLY we apply the name
        // to current constraint and remove NAME_ONLY.
        // Exception is DEFERRABLE_ONLY.
        if (this->constraints.size() > 0 &&
            this->constraints.last()->type == SqliteCreateTable::Column::Constraint::NAME_ONLY &&
            constr->type != SqliteCreateTable::Column::Constraint::DEFERRABLE_ONLY)
        {
            constr->name = this->constraints.last()->name;
            delete this->constraints.takeLast();
        }

        // And the opposite of above. Now we apply DEFERRABLE_ONLY,
        // but only if last item in the list is not NAME_ONLY.
        if (constr->type == SqliteCreateTable::Column::Constraint::DEFERRABLE_ONLY &&
            this->constraints.size() > 0 &&
            this->constraints.last()->type != SqliteCreateTable::Column::Constraint::NAME_ONLY)
        {
            SqliteCreateTable::Column::Constraint* last = this->constraints.last();
            last->deferrable = constr->deferrable;
            last->initially = constr->initially;
            delete constr;

            // We don't want deleted constr to be added to list. We finish this now.
            continue;
        }

        this->constraints << constr;
        constr->setParent(this);
    }
}

SqliteCreateTable::Column::~Column()
{
}

SqliteStatement*SqliteCreateTable::Column::clone()
{
    return new SqliteCreateTable::Column(*this);
}

bool SqliteCreateTable::Column::hasConstraint(SqliteCreateTable::Column::Constraint::Type type) const
{
    return getConstraint(type) != nullptr;
}

SqliteCreateTable::Column::Constraint* SqliteCreateTable::Column::getConstraint(SqliteCreateTable::Column::Constraint::Type type) const
{
    foreach (Constraint* constr, constraints)
        if (constr->type == type)
            return constr;

    return nullptr;
}

QList<SqliteCreateTable::Column::Constraint*> SqliteCreateTable::Column::getForeignKeysByTable(const QString& foreignTable) const
{
    QList<Constraint*> results;
    foreach (Constraint* constr, constraints)
        if (constr->type == Constraint::FOREIGN_KEY && constr->foreignKey->foreignTable.compare(foreignTable, Qt::CaseInsensitive) == 0)
            results << constr;

    return results;
}

QStringList SqliteCreateTable::Column::getColumnsInStatement()
{
    return getStrListFromValue(name);
}

TokenList SqliteCreateTable::Column::getColumnTokensInStatement()
{
    return getTokenListFromNamedKey("columnid");
}

TokenList SqliteCreateTable::Column::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;
    builder.withOther(name, dialect).withStatement(type).withStatementList(constraints, "");
    return builder.build();
}

TokenList SqliteCreateTable::Column::Constraint::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;
    if (!name.isNull())
        builder.withKeyword("CONSTRAINT").withSpace().withOther(name, dialect).withSpace();

    switch (type)
    {
        case SqliteCreateTable::Column::Constraint::PRIMARY_KEY:
        {
            builder.withKeyword("PRIMARY").withSpace().withKeyword("KEY").withSortOrder(sortOrder).withConflict(onConflict);
            if (autoincrKw)
                builder.withSpace().withKeyword("AUTOINCREMENT");

            break;
        }
        case SqliteCreateTable::Column::Constraint::NOT_NULL:
        {
            builder.withKeyword("NOT").withSpace().withKeyword("NULL").withConflict(onConflict);
            break;
        }
        case SqliteCreateTable::Column::Constraint::UNIQUE:
        {
            builder.withKeyword("UNIQUE").withConflict(onConflict);
            break;
        }
        case SqliteCreateTable::Column::Constraint::CHECK:
        {
            builder.withKeyword("CHECK").withSpace().withParLeft().withStatement(expr).withParRight().withConflict(onConflict);
            break;
        }
        case SqliteCreateTable::Column::Constraint::DEFAULT:
        {
            builder.withKeyword("DEFAULT").withSpace();
            if (!id.isNull())
                builder.withOther(id);
            else if (!ctime.isNull())
                builder.withKeyword(ctime.toUpper());
            else if (expr)
                builder.withParLeft().withStatement(expr).withParRight();
            else if (literalNull)
                builder.withKeyword("NULL");
            else
                builder.withLiteralValue(literalValue);

            break;
        }
        case SqliteCreateTable::Column::Constraint::COLLATE:
        {
            builder.withKeyword("COLLATE").withSpace().withOther(collationName, dialect);
            break;
        }
        case SqliteCreateTable::Column::Constraint::FOREIGN_KEY:
        {
            builder.withStatement(foreignKey);
            break;
        }
        case SqliteCreateTable::Column::Constraint::NULL_:
        case SqliteCreateTable::Column::Constraint::NAME_ONLY:
        case SqliteCreateTable::Column::Constraint::DEFERRABLE_ONLY:
            break;
    }

    return builder.build();
}
