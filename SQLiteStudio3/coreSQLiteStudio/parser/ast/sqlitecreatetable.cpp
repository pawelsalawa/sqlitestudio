#include "sqlitecreatetable.h"
#include "parser/parser_helper_stubs.h"
#include "parser/statementtokenbuilder.h"
#include "common/global.h"

const QRegularExpression SqliteCreateTable::Column::GENERATED_ALWAYS_REGEXP = QRegularExpression("GENERATED\\s+ALWAYS");

SqliteCreateTable::SqliteCreateTable()
{
    queryType = SqliteQueryType::CreateTable;
}

SqliteCreateTable::SqliteCreateTable(const SqliteCreateTable& other) :
    SqliteQuery(other), ifNotExistsKw(other.ifNotExistsKw), tempKw(other.tempKw), temporaryKw(other.temporaryKw),
    database(other.database), table(other.table), withOutRowId(other.withOutRowId), strict(other.strict)
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
    for (Column* column : columns)
        column->setParent(this);

    for (SqliteCreateTable::Constraint* constr : constraints)
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

SqliteCreateTable::SqliteCreateTable(bool ifNotExistsKw, int temp, const QString& name1, const QString& name2, const QList<SqliteCreateTable::Column*>& columns, const QList<SqliteCreateTable::Constraint*>& constraints, const QList<ParserStubCreateTableOption*>& options) :
    SqliteCreateTable(ifNotExistsKw, temp, name1, name2, columns, constraints)
{
    this->withOutRowId = parserStubFindCreateTableOption(options, ParserStubCreateTableOption::WITHOUT_ROWID) != nullptr;
    this->strict = parserStubFindCreateTableOption(options, ParserStubCreateTableOption::STRICT) != nullptr;
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
    for (Constraint* constr : constraints)
        if (constr->type == type)
            results << constr;

    return results;
}

SqliteStatement* SqliteCreateTable::getPrimaryKey() const
{
    for (Constraint*& constr : getConstraints(Constraint::PRIMARY_KEY))
        return constr;

    Column::Constraint* colConstr = nullptr;
    for (Column* col : columns)
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
        for (SqliteIndexedColumn* idxCol : tableConstr->indexedColumns)
            colNames << idxCol->name;
    }
    return colNames;
}

SqliteCreateTable::Column* SqliteCreateTable::getColumn(const QString& colName)
{
    for (Column*& col : columns)
    {
        if (col->name.compare(colName, Qt::CaseInsensitive) == 0)
            return col;
    }
    return nullptr;
}

QList<SqliteCreateTable::Constraint*> SqliteCreateTable::getForeignKeysByTable(const QString& foreignTable) const
{
    QList<Constraint*> results;
    for (Constraint* constr : constraints)
        if (constr->type == Constraint::FOREIGN_KEY && constr->foreignKey->foreignTable.compare(foreignTable, Qt::CaseInsensitive) == 0)
            results << constr;

    return results;
}

QList<SqliteCreateTable::Column::Constraint*> SqliteCreateTable::getColumnForeignKeysByTable(const QString& foreignTable) const
{
    QList<Column::Constraint*> results;
    for (Column* col : columns)
        results += col->getForeignKeysByTable(foreignTable);

    return results;
}

void SqliteCreateTable::removeColumnConstraint(Column::Constraint* constr)
{
    for (Column* col : columns)
    {
        if (col->constraints.contains(constr))
        {
            col->constraints.removeOne(constr);
            return;
        }
    }
}

QStringList SqliteCreateTable::getColumnNames() const
{
    QStringList names;
    for (Column* col : columns)
        names << col->name;

    return names;
}

QHash<QString, QString> SqliteCreateTable::getModifiedColumnsMap(bool lowercaseKeys, Qt::CaseSensitivity cs) const
{
    QHash<QString, QString> colMap;
    QString key;
    for (Column* col : columns)
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
    if (!database.isNull())
        builder.withOther(database).withOperator(".");

    builder.withOther(table);

    if (select)
        builder.withSpace().withKeyword("AS").withSpace().withStatement(select);
    else
    {
        builder.withSpace().withParLeft().withStatementList(columns);
        if (constraints.size() > 0)
            builder.withOperator(",").withStatementList(constraints);

        builder.withParRight();

        bool atLeastOneOption = false;
        if (withOutRowId)
        {
            builder.withSpace().withKeyword("WITHOUT").withSpace().withOther("ROWID");
            atLeastOneOption = true;
        }

        if (strict)
        {
            if (atLeastOneOption)
                builder.withOperator(",");

            builder.withSpace().withOther("STRICT");
            //atLeastOneOption = true; // to uncomment if there are further options down below
        }
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
    autoincrKw(other.autoincrKw), generatedKw(other.generatedKw), literalValue(other.literalValue), literalNull(other.literalNull),
    ctime(other.ctime), id(other.id), collationName(other.collationName), generatedType(other.generatedType),
    deferrable(other.deferrable), initially(other.initially)
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

QString SqliteCreateTable::Column::Constraint::toString(SqliteCreateTable::Column::Constraint::GeneratedType type)
{
    switch (type) {
        case SqliteCreateTable::Column::Constraint::GeneratedType::STORED:
            return "STORED";
        case SqliteCreateTable::Column::Constraint::GeneratedType::VIRTUAL:
            return "VIRTUAL";
        case SqliteCreateTable::Column::Constraint::GeneratedType::null:
            break;
    }
    return QString();
}

SqliteCreateTable::Column::Constraint::GeneratedType SqliteCreateTable::Column::Constraint::generatedTypeFrom(const QString& type)
{
    QString upType = type.toUpper();
    if (upType == "STORED")
        return GeneratedType::STORED;
    else if (upType == "VIRTUAL")
        return GeneratedType::VIRTUAL;
    else
        return GeneratedType::null;
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

    for (SqliteIndexedColumn* idxCol : indexedColumns)
        idxCol->setParent(fk);

    for (SqliteForeignKey::Condition* cond : conditions)
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

void SqliteCreateTable::Column::Constraint::initGeneratedAs(SqliteExpr* expr, bool genKw, const QString& type)
{
    this->type = SqliteCreateTable::Column::Constraint::GENERATED;
    this->expr = expr;
    this->generatedKw = genKw;
    this->generatedType = generatedTypeFrom(type);
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
        case SqliteCreateTable::Column::Constraint::GENERATED:
            return "GENERATED";
        case SqliteCreateTable::Column::Constraint::COLLATE:
            return "COLLATE";
        case SqliteCreateTable::Column::Constraint::FOREIGN_KEY:
            return "FOREIGN KEY";
        case SqliteCreateTable::Column::Constraint::NULL_:
        case SqliteCreateTable::Column::Constraint::NAME_ONLY:
        case SqliteCreateTable::Column::Constraint::DEFERRABLE_ONLY:
            break;
    }
    return QString();
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

    for (SqliteIndexedColumn* idxCol : indexedColumns)
        idxCol->setParent(this);
}

void SqliteCreateTable::Constraint::initUnique(const QList<SqliteIndexedColumn *> &indexedColumns, SqliteConflictAlgo algo)
{
    this->type = SqliteCreateTable::Constraint::UNIQUE;
    this->indexedColumns = indexedColumns;
    onConflict = algo;

    for (SqliteIndexedColumn* idxCol : indexedColumns)
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

    for (SqliteIndexedColumn* idxCol : indexedColumns)
        idxCol->setParent(this);

    SqliteForeignKey* fk = new SqliteForeignKey();
    fk->foreignTable = table;
    fk->indexedColumns = fkColumns;
    fk->conditions = conditions;
    fk->deferrable = deferrable;
    fk->initially = initially;

    fk->setParent(this);

    for (SqliteIndexedColumn* idxCol : fkColumns)
        idxCol->setParent(fk);

    for (SqliteForeignKey::Condition* cond : conditions)
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
    for (SqliteIndexedColumn*& idxCol : indexedColumns)
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
            return QString();
    }
    return QString();
}

TokenList SqliteCreateTable::Constraint::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;

    if (!name.isNull())
        builder.withKeyword("CONSTRAINT").withSpace().withOther(name).withSpace();

    switch (type)
    {
        case SqliteCreateTable::Constraint::PRIMARY_KEY:
        {
            builder.withKeyword("PRIMARY").withSpace().withKeyword("KEY").withSpace().withParLeft().withStatementList(indexedColumns);

            if (autoincrKw)
                builder.withSpace().withKeyword("AUTOINCREMENT");

            builder.withParRight().withConflict(onConflict);
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

    for (SqliteCreateTable::Column::Constraint* constr : constraints)
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
            last->foreignKey->deferrable = constr->deferrable;
            last->foreignKey->initially = constr->initially;
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
    for (Constraint* constr : constraints)
        if (constr->type == type)
            return constr;

    return nullptr;
}

QList<SqliteCreateTable::Column::Constraint*> SqliteCreateTable::Column::getConstraints(SqliteCreateTable::Column::Constraint::Type type) const
{
    QList<Constraint*> list;
    for (Constraint* constr : constraints)
        if (constr->type == type)
            list << constr;

    return list;
}

QList<SqliteCreateTable::Column::Constraint*> SqliteCreateTable::Column::getForeignKeysByTable(const QString& foreignTable) const
{
    QList<Constraint*> results;
    for (Constraint* constr : constraints)
        if (constr->type == Constraint::FOREIGN_KEY && constr->foreignKey->foreignTable.compare(foreignTable, Qt::CaseInsensitive) == 0)
            results << constr;

    return results;
}

void SqliteCreateTable::Column::fixTypeVsGeneratedAs()
{
    // This is a workaround for lemon parser taking "GENERATED ALWAYS" as part of the typename,
    // despite 2 days effort of forcing proper precedense to parse it as part of a constraint.
    // Lemon keeps reducing these 2 keywords into the typename by using fallback of GENERATED & ALWAYS to ID,
    // regardless of rule order and explicit precedence. By throwing the GENERATED keyword out of the fallback list,
    // we would make the syntax incompatible with official SQLite syntax, which allows usage of GENERATED as ID.
    // I've tried to use more recent Lemon parser, but it's different a lot from current one and it is no longer possible
    // to collect tokens parsed per rule (needed tor SqliteStatement's tokens & tokenMap). At least not in the way
    // that it used to be so far.
    // This is the last resort to make it right.
    // This method is called from parser rule that reduces column definition (rule "column(X)").
    Constraint* generatedConstr = getConstraint(Constraint::GENERATED);
    if (generatedConstr && !generatedConstr->generatedKw && type && type->name.toUpper().contains(GENERATED_ALWAYS_REGEXP))
    {
        type->name.replace(GENERATED_ALWAYS_REGEXP, "");
        type->tokens = type->rebuildTokensFromContents();
        type->tokensMap["typename"] = type->tokens;
        generatedConstr->generatedKw = true;
    }
}

void SqliteCreateTable::Column::evaluatePostParsing()
{
    fixTypeVsGeneratedAs();
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
    builder.withOther(name).withStatement(type).withStatementList(constraints, "");
    return builder.build();
}

TokenList SqliteCreateTable::Column::Constraint::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;
    if (!name.isNull())
        builder.withKeyword("CONSTRAINT").withSpace().withOther(name).withSpace();

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
            builder.withKeyword("COLLATE").withSpace().withOther(collationName);
            break;
        }
        case SqliteCreateTable::Column::Constraint::FOREIGN_KEY:
        {
            builder.withStatement(foreignKey);
            break;
        }
        case SqliteCreateTable::Column::Constraint::GENERATED:
        {
            if (generatedKw)
                builder.withKeyword("GENERATED").withSpace().withKeyword("ALWAYS").withSpace();

            builder.withKeyword("AS").withSpace().withParLeft().withStatement(expr).withParRight();
            if (generatedType != GeneratedType::null)
                builder.withSpace().withOther(toString(generatedType), false);

            break;
        }
        case SqliteCreateTable::Column::Constraint::NULL_:
        case SqliteCreateTable::Column::Constraint::NAME_ONLY:
        case SqliteCreateTable::Column::Constraint::DEFERRABLE_ONLY:
            break;
    }

    return builder.build();
}

QString SqliteCreateTable::getTargetDatabase() const
{
    return database;
}

void SqliteCreateTable::setTargetDatabase(const QString& database)
{
    this->database = database;
}

QString SqliteCreateTable::getObjectName() const
{
    return table;
}

void SqliteCreateTable::setObjectName(const QString& name)
{
    table = name;
}
