#include "sqliteselect.h"
#include "sqlitequerytype.h"
#include "parser/statementtokenbuilder.h"
#include "common/global.h"
#include "sqlitewith.h"
#include "sqlitewindowdefinition.h"
#include <QSet>

SqliteSelect::SqliteSelect()
{
    queryType = SqliteQueryType::Select;
}

SqliteSelect::SqliteSelect(const SqliteSelect& other) :
    SqliteQuery(other)
{
    DEEP_COPY_COLLECTION(Core, coreSelects);
    DEEP_COPY_FIELD(SqliteWith, with);
}

SqliteSelect* SqliteSelect::append(Core* core)
{
    SqliteSelect* select = new SqliteSelect();
    select->coreSelects << core;
    core->setParent(select);
    return select;
}

SqliteSelect* SqliteSelect::append(SqliteSelect* select, SqliteSelect::CompoundOperator op, Core* core)
{
    if (!select)
        select = new SqliteSelect();

    core->compoundOp = op;
    select->coreSelects << core;
    core->setParent(select);
    return select;
}

SqliteSelect* SqliteSelect::append(const QList<QList<SqliteExpr*>>& values)
{
    return append(nullptr, CompoundOperator::null, values);
}

SqliteSelect* SqliteSelect::append(SqliteSelect* select, SqliteSelect::CompoundOperator op, const QList<QList<SqliteExpr*>>& values)
{
    if (!select)
        select = new SqliteSelect();

    bool first = true;

    Core::ResultColumn* resCol = nullptr;
    QList<Core::ResultColumn*> resColList;
    for (const QList<SqliteExpr*>& singleValues : values)
    {
        Core* core = new Core();
        core->setParent(select);
        core->compoundOp = op;
        core->valuesMode = true;
        if (first)
        {
            op = CompoundOperator::UNION_ALL;
            first = false;
        }
        select->coreSelects << core;

        resColList.clear();
        for (SqliteExpr* value : singleValues)
        {
            resCol = new Core::ResultColumn(value, false, QString());
            value->detectDoubleQuotes(); // invoke explicitly before rebuilding tokens not to lose this information
            resCol->rebuildTokens();
            resCol->setParent(core);
            core->resultColumns << resCol;
        }
    }

    return select;
}

SqliteStatement*SqliteSelect::clone()
{
    return new SqliteSelect(*this);
}

QString SqliteSelect::compoundOperator(SqliteSelect::CompoundOperator op)
{
    switch (op)
    {
        case SqliteSelect::CompoundOperator::UNION:
            return "UNION";
        case SqliteSelect::CompoundOperator::UNION_ALL:
            return "UNION ALL";
        case SqliteSelect::CompoundOperator::INTERSECT:
            return "INTERSECT";
        case SqliteSelect::CompoundOperator::EXCEPT:
            return "EXCEPT";
        case SqliteSelect::CompoundOperator::null:
            break;
    }
    return QString();
}

SqliteSelect::CompoundOperator SqliteSelect::compoundOperator(const QString& op)
{
    QString upStr = op.toUpper();
    if (upStr == "UNION")
        return CompoundOperator::UNION;
    else if (upStr == "UNION ALL")
        return CompoundOperator::UNION_ALL;
    else if (upStr == "EXCEPT")
        return CompoundOperator::EXCEPT;
    else if (upStr == "INTERSECT")
        return CompoundOperator::INTERSECT;
    else
        return CompoundOperator::null;
}

void SqliteSelect::reset()
{
    for (Core*& core : coreSelects)
        delete core;

    coreSelects.clear();
}

void SqliteSelect::setWith(SqliteWith* with)
{
    this->with = with;
    if (with)
        with->setParent(this);
}

SqliteSelect::Core::Core()
{
}

SqliteSelect::Core::Core(const SqliteSelect::Core& other) :
    SqliteStatement(other), compoundOp(other.compoundOp), distinctKw(other.distinctKw), allKw(other.allKw)
{
    DEEP_COPY_COLLECTION(ResultColumn, resultColumns);
    DEEP_COPY_FIELD(JoinSource, from);
    DEEP_COPY_FIELD(SqliteExpr, where);
    DEEP_COPY_FIELD(SqliteExpr, having);
    DEEP_COPY_COLLECTION(SqliteExpr, groupBy);
    DEEP_COPY_COLLECTION(SqliteWindowDefinition, windows);
    DEEP_COPY_COLLECTION(SqliteOrderBy, orderBy);
    DEEP_COPY_FIELD(SqliteLimit, limit);
}

SqliteSelect::Core::Core(int distinct, const QList<ResultColumn*>& resCols, JoinSource* src,
                         SqliteExpr* where, const QList<SqliteExpr*>& groupBy, SqliteExpr* having,
                         const QList<SqliteOrderBy*>& orderBy, SqliteLimit* limit) :
    Core(distinct, resCols, src, where, groupBy, having, QList<SqliteWindowDefinition*>(), orderBy, limit)
{
}

SqliteSelect::Core::Core(int distinct, const QList<ResultColumn *> &resCols, JoinSource *src,
                         SqliteExpr *where, const QList<SqliteExpr *> &groupBy, SqliteExpr *having, const QList<SqliteWindowDefinition*> windows,
                         const QList<SqliteOrderBy*>& orderBy, SqliteLimit* limit)
{
    if (distinct == 1)
        distinctKw = true;
    else if (distinct == 2)
        allKw = true;

    from = src;
    this->where = where;
    this->having = having;
    this->windows = windows;
    this->groupBy = groupBy;
    resultColumns = resCols;
    this->limit = limit;
    this->orderBy = orderBy;

    if (from)
        from->setParent(this);

    if (where)
        where->setParent(this);

    if (having)
        having->setParent(this);

    if (limit)
        limit->setParent(this);

    for (SqliteWindowDefinition* win : windows)
        win->setParent(this);

    for (SqliteOrderBy* order : orderBy)
        order->setParent(this);

    for (SqliteExpr* expr : groupBy)
        expr->setParent(this);

    for (SqliteSelect::Core::ResultColumn* resCol : resCols)
        resCol->setParent(this);
}

SqliteStatement*SqliteSelect::Core::clone()
{
    return new SqliteSelect::Core(*this);
}

SqliteSelect::Core::ResultColumn::ResultColumn()
{
}

SqliteSelect::Core::ResultColumn::ResultColumn(const SqliteSelect::Core::ResultColumn& other) :
    SqliteStatement(other), star(other.star), asKw(other.asKw), alias(other.alias), table(other.table)
{
    DEEP_COPY_FIELD(SqliteExpr, expr);
}

SqliteSelect::Core::ResultColumn::ResultColumn(SqliteExpr *expr, bool asKw, const QString &alias)
{
    this->expr = expr;
    this->asKw = asKw;
    this->alias = alias;
    if (expr)
        expr->setParent(this);
}

SqliteSelect::Core::ResultColumn::ResultColumn(bool star, const QString &table)
{
    this->star = star;
    this->table = table;
}

SqliteSelect::Core::ResultColumn::ResultColumn(bool star)
{
    this->star = star;
}

bool SqliteSelect::Core::ResultColumn::isRowId()
{
    if (!expr)
        return false;

    if (expr->column.isEmpty())
        return false;

    return expr->column.compare("rowid", Qt::CaseInsensitive) == 0;
}

SqliteStatement*SqliteSelect::Core::ResultColumn::clone()
{
    return new SqliteSelect::Core::ResultColumn(*this);
}

QStringList SqliteSelect::Core::ResultColumn::getTablesInStatement()
{
    return getStrListFromValue(table);
}

TokenList SqliteSelect::Core::ResultColumn::getTableTokensInStatement()
{
    // To avoid warnings about missing "nm" key
    if (table.isNull())
        return TokenList();

    // Now, we know table was specified
    return getTokenListFromNamedKey("tnm");
}

QList<SqliteStatement::FullObject> SqliteSelect::Core::ResultColumn::getFullObjectsInStatement()
{
    QList<FullObject> result;

    // Table object
    TokenList tableTokens = getTableTokensInStatement();
    FullObject fullObj;
    if (tableTokens.size() > 0)
        fullObj = getFullObject(FullObject::TABLE, dbTokenForFullObjects, tableTokens[0]);

    if (fullObj.isValid())
        result << fullObj;

    return result;
}

SqliteSelect::Core::SingleSource::SingleSource()
{
}

SqliteSelect::Core::SingleSource::SingleSource(const SqliteSelect::Core::SingleSource& other) :
    SqliteStatement(other), database(other.database), table(other.table), alias(other.alias), asKw(other.asKw), indexedByKw(other.indexedByKw),
    notIndexedKw(other.notIndexedKw), indexedBy(other.indexedBy)
{
    DEEP_COPY_FIELD(SqliteSelect, select);
    DEEP_COPY_FIELD(JoinSource, joinSource);
}

SqliteSelect::Core::SingleSource::SingleSource(const QString& name1, const QString& name2, bool asKw, const QString& alias, bool notIndexedKw, const QString& indexedBy)
{
    if (!name2.isNull())
    {
        database = name1;
        table = name2;
    }
    else
        table = name1;

    this->asKw = asKw;
    this->alias = alias;
    this->indexedBy = indexedBy;
    this->indexedByKw = !(indexedBy.isNull());
    this->notIndexedKw = notIndexedKw;
}

SqliteSelect::Core::SingleSource::SingleSource(const QString &name1, const QString &name2, bool asKw, const QString &alias, const QList<SqliteExpr*> &exprList)
{
    if (!name2.isNull())
    {
        database = name1;
        funcName = name2;
    }
    else
        funcName = name1;

    funcParams.append(exprList);
    for (SqliteExpr* expr : exprList)
        expr->setParent(this);

    this->asKw = asKw;
    this->alias = alias;
}

SqliteSelect::Core::SingleSource::SingleSource(SqliteSelect *select, bool asKw, const QString &alias)
{
    this->select = select;
    this->asKw = asKw;
    this-> alias = alias;
    if (select)
        select->setParent(this);
}

SqliteSelect::Core::SingleSource::SingleSource(JoinSource *src, bool asKw, const QString &alias)
{
    this->joinSource = src;
    this->asKw = asKw;
    this-> alias = alias;
    if (src)
        src->setParent(this);
}

SqliteStatement*SqliteSelect::Core::SingleSource::clone()
{
    return new SqliteSelect::Core::SingleSource(*this);
}

QStringList SqliteSelect::Core::SingleSource::getTablesInStatement()
{
    // This method returns tables only! No aliases.
    // Otherwise the completion sorter won't work correctly.
    // To return tables with aliases use/create other method.
    return getStrListFromValue(table);
}

QStringList SqliteSelect::Core::SingleSource::getDatabasesInStatement()
{
    return getStrListFromValue(database);
}

TokenList SqliteSelect::Core::SingleSource::getTableTokensInStatement()
{
    if (table.isNull())
        return TokenList();

    return getObjectTokenListFromNmDbnm();
}

TokenList SqliteSelect::Core::SingleSource::getDatabaseTokensInStatement()
{
    if (database.isNull())
        return TokenList();

    return getDbTokenListFromNmDbnm();
}

QList<SqliteStatement::FullObject> SqliteSelect::Core::SingleSource::getFullObjectsInStatement()
{
    QList<FullObject> result;

    // Table object
    if (!table.isNull())
    {
        FullObject fullObj = getFullObjectFromNmDbnm(FullObject::TABLE);

        if (fullObj.isValid())
            result << fullObj;
    }

    // Db object
    if (!database.isNull())
    {
        FullObject fullObj = getFirstDbFullObject();
        if (fullObj.isValid())
        {
            result << fullObj;
            dbTokenForFullObjects = fullObj.database;
        }
    }

    return result;
}

SqliteSelect::Core::JoinConstraint::JoinConstraint()
{
}

SqliteSelect::Core::JoinConstraint::JoinConstraint(const SqliteSelect::Core::JoinConstraint& other) :
    SqliteStatement(other), columnNames(other.columnNames)
{
    DEEP_COPY_FIELD(SqliteExpr, expr);
}

SqliteSelect::Core::JoinConstraint::JoinConstraint(SqliteExpr *expr)
{
    this->expr = expr;
    if (expr)
        expr->setParent(this);
}

SqliteSelect::Core::JoinConstraint::JoinConstraint(const QList<QString> &strList)
{
    columnNames = strList;
}

SqliteStatement*SqliteSelect::Core::JoinConstraint::clone()
{
    return new SqliteSelect::Core::JoinConstraint(*this);
}

QStringList SqliteSelect::Core::JoinConstraint::getColumnsInStatement()
{
    QStringList list;
    list += columnNames;
    return list;
}

TokenList SqliteSelect::Core::JoinConstraint::getColumnTokensInStatement()
{
    TokenList list;
    for (TokenPtr& token : getTokenListFromNamedKey("idlist", -1))
    {
        if (token->type == Token::OPERATOR) // a COMMA
            continue;

        list << token;
    }
    return list;
}

SqliteSelect::Core::JoinOp::JoinOp()
{
}

SqliteSelect::Core::JoinOp::JoinOp(const SqliteSelect::Core::JoinOp& other) :
    SqliteStatement(other), comma(other.comma), joinKw(other.joinKw), naturalKw(other.naturalKw), leftKw(other.leftKw), outerKw(other.outerKw),
    innerKw(other.innerKw), crossKw(other.crossKw), rightKw(other.rightKw), fullKw(other.fullKw), customKw1(other.customKw1),
    customKw2(other.customKw2), customKw3(other.customKw3)
{
}

SqliteSelect::Core::JoinOp::JoinOp(bool comma)
{
    this->comma = comma;
    this->joinKw = !comma;
}

SqliteSelect::Core::JoinOp::JoinOp(const QString &joinToken)
{
    this->joinKw = true;
    init(joinToken);
}

SqliteSelect::Core::JoinOp::JoinOp(const QString &joinToken, const QString &word1)
{
    this->joinKw = true;
    init(joinToken);
    init(word1);
}

SqliteSelect::Core::JoinOp::JoinOp(const QString &joinToken, const QString &word1, const QString &word2)
{
    this->joinKw = true;
    init(joinToken);
    init(word1);
    init(word2);
}

SqliteStatement*SqliteSelect::Core::JoinOp::clone()
{
    return new SqliteSelect::Core::JoinOp(*this);
}

void SqliteSelect::Core::JoinOp::init(const QString &str)
{
    QString upStr = str.toUpper();
    if (upStr == "NATURAL")
        naturalKw = true;
    else if (upStr == "LEFT")
        leftKw = true;
    else if (upStr == "RIGHT")
        rightKw = true;
    else if (upStr == "FULL")
        fullKw = true;
    else if (upStr == "OUTER")
        outerKw = true;
    else if (upStr == "INNER")
        innerKw = true;
    else if (upStr == "CROSS")
        crossKw = true;
    else if (customKw1.isNull())
        customKw1 = str;
    else if (customKw2.isNull())
        customKw2 = str;
    else
        customKw3 = str;
}


SqliteSelect::Core::JoinSourceOther::JoinSourceOther()
{
}

SqliteSelect::Core::JoinSourceOther::JoinSourceOther(const SqliteSelect::Core::JoinSourceOther& other) :
    SqliteStatement(other)
{
    DEEP_COPY_FIELD(JoinOp, joinOp);
    DEEP_COPY_FIELD(SingleSource, singleSource);
    DEEP_COPY_FIELD(JoinConstraint, joinConstraint);
}

SqliteSelect::Core::JoinSourceOther::JoinSourceOther(SqliteSelect::Core::JoinOp *op, SingleSource *src, JoinConstraint *constr)
{
    this->joinConstraint = constr;
    this->joinOp = op;
    this->singleSource = src;
    if (constr)
        constr->setParent(this);

    if (op)
        op->setParent(this);

    if (src)
        src->setParent(this);
}

SqliteStatement*SqliteSelect::Core::JoinSourceOther::clone()
{
    return new SqliteSelect::Core::JoinSourceOther(*this);
}


SqliteSelect::Core::JoinSource::JoinSource()
{
}

SqliteSelect::Core::JoinSource::JoinSource(const JoinSource& other) :
    SqliteStatement(other)
{
    DEEP_COPY_FIELD(SingleSource, singleSource);
    DEEP_COPY_COLLECTION(JoinSourceOther, otherSources);
}

SqliteSelect::Core::JoinSource::JoinSource(SqliteSelect::Core::SingleSource *singleSource, const QList<SqliteSelect::Core::JoinSourceOther *> &list)
{
    this->singleSource = singleSource;
    this->otherSources = list;
    if (singleSource)
        singleSource->setParent(this);

    for (JoinSourceOther*& other : otherSources)
        other->setParent(this);
}

SqliteStatement*SqliteSelect::Core::JoinSource::clone()
{
    return new SqliteSelect::Core::JoinSource(*this);
}


TokenList SqliteSelect::Core::ResultColumn::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;
    if (star)
    {
        if (!table.isNull())
            builder.withOther(table).withOperator(".");

        builder.withOperator("*");
    }
    else
    {
        builder.withStatement(expr);
        if (!alias.isNull())
        {
            if (asKw)
                builder.withSpace().withKeyword("AS");

            builder.withSpace().withOther(alias);
        }
    }

    return builder.build();
}

TokenList SqliteSelect::Core::SingleSource::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;
    if (!table.isNull())
    {
        if (!database.isNull())
            builder.withOther(database).withOperator(".");

        builder.withOther(table);

        if (!alias.isNull())
        {
            if (asKw)
                builder.withSpace().withKeyword("AS");

            builder.withSpace().withOther(alias);
        }
    }
    else if (!funcName.isNull())
    {
        if (!database.isNull())
            builder.withOther(database).withOperator(".");

        builder.withOther(funcName).withParLeft().withStatementList(funcParams).withParRight();

        if (!alias.isNull())
        {
            if (asKw)
                builder.withSpace().withKeyword("AS");

            builder.withSpace().withOther(alias);
        }

        if (indexedByKw)
            builder.withSpace().withKeyword("INDEXED").withSpace().withKeyword("BY").withSpace().withOther(indexedBy);
        else if (notIndexedKw)
            builder.withSpace().withKeyword("NOT").withSpace().withKeyword("INDEXED");
    }
    else if (select)
    {
        builder.withParLeft().withStatement(select).withParRight();
        if (!alias.isNull())
        {
            if (asKw)
                builder.withSpace().withKeyword("AS");

            builder.withSpace().withOther(alias);
        }
    }
    else
    {
        builder.withParLeft().withStatement(joinSource).withParRight();
    }

    return builder.build();
}

TokenList SqliteSelect::Core::JoinOp::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;
    if (comma)
    {
        builder.withOperator(",");
    }
    else
    {
        if (naturalKw)
            builder.withKeyword("NATURAL").withSpace();

        if (leftKw || fullKw || rightKw)
        {
            if (leftKw)
                builder.withKeyword("LEFT");
            else if (fullKw)
                builder.withKeyword("FULL");
            else if (rightKw)
                builder.withKeyword("RIGHT");

            builder.withSpace();
            if (outerKw)
                builder.withKeyword("OUTER").withSpace();
        }
        else if (innerKw)
            builder.withKeyword("INNER").withSpace();
        else if (crossKw)
            builder.withKeyword("CROSS").withSpace();

        builder.withKeyword("JOIN");
    }

    return builder.build();
}


TokenList SqliteSelect::Core::JoinConstraint::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;
    if (expr)
        builder.withKeyword("ON").withStatement(expr);
    else
        builder.withKeyword("USING").withSpace().withParLeft().withOtherList(columnNames).withParRight();

    return builder.build();
}


TokenList SqliteSelect::Core::JoinSourceOther::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;
    builder.withStatement(joinOp).withStatement(singleSource).withStatement(joinConstraint);
    return builder.build();
}


TokenList SqliteSelect::Core::JoinSource::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;
    builder.withStatement(singleSource).withStatementList(otherSources, "");
    return builder.build();
}


TokenList SqliteSelect::Core::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;
    if (valuesMode)
    {
        SqliteSelect* select = dynamic_cast<SqliteSelect*>(parentStatement());
        if (select->coreSelects.indexOf(this) == 0) // this is first core in series of cores of values mode of the SELECT
            builder.withKeyword("VALUES").withSpace();

        builder.withParLeft().withStatementList(resultColumns).withParRight();
        return builder.build();
    }

    builder.withKeyword("SELECT");
    if (distinctKw)
        builder.withSpace().withKeyword("DISTINCT");
    else if (allKw)
        builder.withSpace().withKeyword("ALL");

    builder.withStatementList(resultColumns);
    if (from)
        builder.withSpace().withKeyword("FROM").withStatement(from);

    if (where)
        builder.withSpace().withKeyword("WHERE").withStatement(where);

    if (groupBy.size() > 0)
    {
        builder.withSpace().withKeyword("GROUP").withSpace().withKeyword("BY").withStatementList(groupBy);
        if (having)
            builder.withSpace().withKeyword("HAVING").withStatement(having);
    }

    if (windows.size() > 0)
        builder.withSpace().withKeyword("WINDOW").withStatementList(windows);

    if (orderBy.size() > 0)
        builder.withSpace().withKeyword("ORDER").withSpace().withKeyword("BY").withStatementList(orderBy);

    if (limit)
        builder.withStatement(limit);

    return builder.build();
}

TokenList SqliteSelect::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;
    builder.withTokens(SqliteQuery::rebuildTokensFromContents());
    if (with)
        builder.withStatement(with);

    for (SqliteSelect::Core*& core : coreSelects)
    {
        if (core->compoundOp == CompoundOperator::UNION_ALL)
        {
            if (core->valuesMode)
                builder.withSpace().withOperator(",");
            else
                builder.withSpace().withKeyword("UNION").withSpace().withKeyword("ALL");
        }
        else if (core->compoundOp != CompoundOperator::null)
            builder.withSpace().withKeyword(compoundOperator(core->compoundOp));

        builder.withStatement(core);
    }

    builder.withOperator(";");

    return builder.build();
}
