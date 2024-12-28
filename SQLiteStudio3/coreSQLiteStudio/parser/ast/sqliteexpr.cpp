#include "sqliteexpr.h"
#include "sqliteraise.h"
#include "sqliteselect.h"
#include "sqlitecolumntype.h"
#include "sqlitefilterover.h"
#include "parser/statementtokenbuilder.h"
#include "common/utils_sql.h"
#include "common/global.h"
#include <QDebug>

SqliteExpr::SqliteExpr()
{
}

SqliteExpr::SqliteExpr(const SqliteExpr& other) :
    SqliteStatement(other),
    mode(other.mode), literalValue(other.literalValue), literalNull(other.literalNull), bindParam(other.bindParam), database(other.database), table(other.table),
    column(other.column), unaryOp(other.unaryOp), binaryOp(other.binaryOp), function(other.function), collation(other.collation),
    ctime(other.ctime), distinctKw(other.distinctKw), allKw(other.allKw), star(other.star), notKw(other.notKw), like(other.like),
    notNull(other.notNull), possibleDoubleQuotedString(other.possibleDoubleQuotedString)
{
    DEEP_COPY_FIELD(SqliteColumnType, columnType);
    DEEP_COPY_FIELD(SqliteExpr, expr1);
    DEEP_COPY_FIELD(SqliteExpr, expr2);
    DEEP_COPY_FIELD(SqliteExpr, expr3);
    DEEP_COPY_COLLECTION(SqliteExpr, exprList);
    DEEP_COPY_FIELD(SqliteSelect, select);
    DEEP_COPY_FIELD(SqliteRaise, raiseFunction);
    DEEP_COPY_FIELD(SqliteFilterOver, filterOver);
}

SqliteExpr::~SqliteExpr()
{
}

SqliteExpr::LikeOp SqliteExpr::likeOp(const QString &value)
{
    QString upper = value.toUpper();
    if (upper == "LIKE")
        return SqliteExpr::LikeOp::LIKE;
    else if (upper == "GLOB")
        return SqliteExpr::LikeOp::GLOB;
    else if (upper == "REGEXP")
        return SqliteExpr::LikeOp::REGEXP;
    else if (upper == "MATCH")
        return SqliteExpr::LikeOp::MATCH;
    else
        return SqliteExpr::LikeOp::null;
}

QString SqliteExpr::likeOp(SqliteExpr::LikeOp value)
{
    switch (value)
    {
        case SqliteExpr::LikeOp::LIKE:
            return "LIKE";
        case SqliteExpr::LikeOp::GLOB:
            return "GLOB";
        case SqliteExpr::LikeOp::REGEXP:
            return "REGEXP";
        case SqliteExpr::LikeOp::MATCH:
            return "MATCH";
        default:
            return QString();
    }
}

SqliteExpr::NotNull SqliteExpr::notNullOp(const QString &value)
{
    QString upper = value.toUpper();
    if (upper == "ISNULL")
        return SqliteExpr::NotNull::ISNULL;
    else if (upper == "NOTNULL")
        return SqliteExpr::NotNull::NOTNULL;
    else if (upper == "NOT NULL")
        return SqliteExpr::NotNull::NOT_NULL;
    else
        return SqliteExpr::NotNull::null;
}

QString SqliteExpr::notNullOp(SqliteExpr::NotNull value)
{
    switch (value)
    {
        case SqliteExpr::NotNull::ISNULL:
            return "ISNULL";
        case SqliteExpr::NotNull::NOT_NULL:
            return "NOT NULL";
        case SqliteExpr::NotNull::NOTNULL:
            return "NOTNULL";
        default:
            return QString();
    }
}

SqliteStatement* SqliteExpr::clone()
{
    return new SqliteExpr(*this);
}

void SqliteExpr::initLiteral(const QVariant &value)
{
    mode = SqliteExpr::Mode::LITERAL_VALUE;
    if (value.isNull())
        initNull();

    literalValue = value;
}

void SqliteExpr::initNull()
{
    literalNull = true;
}

void SqliteExpr::initCTime(const QString &name)
{
    mode = SqliteExpr::Mode::CTIME;
    ctime = name;
}

void SqliteExpr::initSubExpr(SqliteExpr *expr)
{
    mode = SqliteExpr::Mode::SUB_EXPR;
    expr1 = expr;
    if (expr)
        expr->setParent(this);
}

void SqliteExpr::initRowValue(const QList<SqliteExpr*> &exprList)
{
    if (exprList.size() == 1)
    {
        initSubExpr(exprList.first());
        return;
    }

    mode = SqliteExpr::Mode::ROW_VALUE;
    this->exprList = exprList;

    for (SqliteExpr* expr : exprList)
        expr->setParent(this);
}

void SqliteExpr::initBindParam(const QString& value)
{
    mode = SqliteExpr::Mode::BIND_PARAM;
    bindParam = value;
}

void SqliteExpr::initCollate(SqliteExpr *expr, const QString& value)
{
    mode = SqliteExpr::Mode::COLLATE;
    expr1 = expr;
    collation = value;
    if (expr)
        expr->setParent(this);
}

void SqliteExpr::initCast(SqliteExpr *expr, SqliteColumnType *type)
{
    mode = SqliteExpr::Mode::CAST;
    expr1 = expr;
    columnType = type;
    if (expr)
        expr->setParent(this);
}

void SqliteExpr::initFunction(const QString& fnName, int distinct, const QList<SqliteExpr*>& exprList)
{
    mode = SqliteExpr::Mode::FUNCTION;
    function = fnName;
    this->exprList = exprList;
    initDistinct(distinct);

    for (SqliteExpr* expr : exprList)
        expr->setParent(this);
}

void SqliteExpr::initFunction(const QString& fnName, bool star)
{
    mode = SqliteExpr::Mode::FUNCTION;
    function = fnName;
    this->star = star;
}

void SqliteExpr::initWindowFunction(const QString& fnName, int distinct, const QList<SqliteExpr*>& exprList, SqliteFilterOver* filterOver)
{
    mode = SqliteExpr::Mode::WINDOW_FUNCTION;
    this->function = fnName;
    this->exprList = exprList;
    initDistinct(distinct);
    this->filterOver = filterOver;

    for (SqliteExpr* expr : exprList)
        expr->setParent(this);

    if (filterOver)
        filterOver->setParent(this);
}

void SqliteExpr::initWindowFunction(const QString& fnName, SqliteFilterOver* filterOver)
{
    mode = SqliteExpr::Mode::WINDOW_FUNCTION;
    this->function = fnName;
    this->star = true;
    this->filterOver = filterOver;

    if (filterOver)
        filterOver->setParent(this);
}

void SqliteExpr::initBinOp(SqliteExpr *expr1, const QString& op, SqliteExpr *expr2)
{
    mode = SqliteExpr::Mode::BINARY_OP;
    this->expr1 = expr1;
    this->expr2 = expr2;
    binaryOp = op;
    if (expr1)
        expr1->setParent(this);

    if (expr2)
        expr2->setParent(this);
}

void SqliteExpr::initUnaryOp(SqliteExpr *expr, const QString& op)
{
    mode = SqliteExpr::Mode::UNARY_OP;
    expr1 = expr;
    unaryOp = op;
    if (expr)
        expr->setParent(this);
}

void SqliteExpr::initPtrOp(SqliteExpr* expr1, const QString& op, SqliteExpr* expr2)
{
    mode = SqliteExpr::Mode::PTR_OP;
    this->expr1 = expr1;
    this->expr2 = expr2;
    ptrOp = op;
    if (expr1)
        expr1->setParent(this);

    if (expr2)
        expr2->setParent(this);
}

void SqliteExpr::initLike(SqliteExpr *expr1, bool notKw, LikeOp likeOp, SqliteExpr *expr2, SqliteExpr *expr3)
{
    mode = SqliteExpr::Mode::LIKE;
    this->expr1 = expr1;
    this->expr2 = expr2;
    this->expr3 = expr3;
    this->notKw = notKw;
    this->like = likeOp;
    if (expr1)
        expr1->setParent(this);

    if (expr2)
        expr2->setParent(this);

    if (expr3)
        expr3->setParent(this);
}

void SqliteExpr::initNull(SqliteExpr *expr, const QString& value)
{
    mode = SqliteExpr::Mode::NOTNULL;
    expr1 = expr;
    notNull = notNullOp(value);
    if (expr)
        expr->setParent(this);
}

void SqliteExpr::initIs(SqliteExpr *expr1, bool notKw, SqliteExpr *expr2)
{
    mode = SqliteExpr::Mode::IS;
    this->expr1 = expr1;
    this->notKw = notKw;
    this->expr2 = expr2;
    if (expr1)
        expr1->setParent(this);

    if (expr2)
        expr2->setParent(this);
}

void SqliteExpr::initDistinct(SqliteExpr* expr1, bool notKw, SqliteExpr* expr2)
{
    mode = SqliteExpr::Mode::DISTINCT;
    this->expr1 = expr1;
    this->notKw = notKw;
    this->expr2 = expr2;
    if (expr1)
        expr1->setParent(this);

    if (expr2)
        expr2->setParent(this);
}

void SqliteExpr::initBetween(SqliteExpr *expr1, bool notKw, SqliteExpr *expr2, SqliteExpr *expr3)
{
    mode = SqliteExpr::Mode::BETWEEN;
    this->expr1 = expr1;
    this->expr2 = expr2;
    this->expr3 = expr3;
    this->notKw = notKw;
    if (expr1)
        expr1->setParent(this);

    if (expr2)
        expr2->setParent(this);

    if (expr3)
        expr3->setParent(this);
}

void SqliteExpr::initIn(SqliteExpr *expr, bool notKw, const QList<SqliteExpr*>& exprList)
{
    mode = SqliteExpr::Mode::IN;
    expr1 = expr;
    this->notKw = notKw;
    this->exprList = exprList;
    for (SqliteExpr* expr : exprList)
        expr->setParent(this);
}

void SqliteExpr::initIn(SqliteExpr *expr, bool notKw, SqliteSelect *select)
{
    mode = SqliteExpr::Mode::IN;
    expr1 = expr;
    this->notKw = notKw;
    this->select = select;
    if (expr)
        expr->setParent(this);

    if (select)
        select->setParent(this);
}

void SqliteExpr::initIn(SqliteExpr *expr, bool notKw, const QString& name1, const QString& name2)
{
    mode = SqliteExpr::Mode::IN;
    expr1 = expr;
    this->notKw = notKw;
    if (!name2.isNull())
    {
        database = name1;
        table = name2;
    }
    else
        table = name1;

    if (expr)
        expr->setParent(this);
}

void SqliteExpr::initExists(SqliteSelect *select)
{
    mode = SqliteExpr::Mode::EXISTS;
    this->select = select;
    if (select)
        select->setParent(this);
}

void SqliteExpr::initSubSelect(SqliteSelect *select)
{
    mode = SqliteExpr::Mode::SUB_SELECT;
    this->select = select;
    if (select)
        select->setParent(this);
}

void SqliteExpr::initCase(SqliteExpr *expr1, const QList<SqliteExpr*>& exprList, SqliteExpr *expr2)
{
    mode = SqliteExpr::Mode::CASE;
    this->expr1 = expr1;
    this->expr2 = expr2;
    this->exprList = exprList;
    if (expr1)
        expr1->setParent(this);

    if (expr2)
        expr2->setParent(this);

    for (SqliteExpr* expr : exprList)
        expr->setParent(this);
}

void SqliteExpr::initRaise(const QString& type, const QString& text)
{
    mode = SqliteExpr::Mode::RAISE;
    raiseFunction = new SqliteRaise(type, text);
}

void SqliteExpr::detectDoubleQuotes(bool recursively)
{
    if (doubleQuotesChecked)
        return;

    doubleQuotesChecked = true;

    if (tokens.size() > 0)
    {
        QString val = tokens.first()->value;
        if (val[0] == '"' && val[0] == val[val.length() - 1])
            possibleDoubleQuotedString = true;
    }

    for (SqliteStatement* stmt : childStatements())
    {
        SqliteExpr* subExpr = dynamic_cast<SqliteExpr*>(stmt);
        if (subExpr)
            subExpr->detectDoubleQuotes(recursively);
    }
}

bool SqliteExpr::replace(SqliteExpr* toBeReplaced, SqliteExpr* replaceWith)
{
    if (expr1 == toBeReplaced)
    {
        expr1 = replaceWith;
        return true;
    }

    if (expr2 == toBeReplaced)
    {
        expr2 = replaceWith;
        return true;
    }

    if (expr3 == toBeReplaced)
    {
        expr3 = replaceWith;
        return true;
    }

    int idx = exprList.indexOf(toBeReplaced);
    if (idx > -1)
    {
        exprList.replace(idx, replaceWith);
        return true;
    }

    return false;
}

QStringList SqliteExpr::getColumnsInStatement()
{
    return getStrListFromValue(column);
}

QStringList SqliteExpr::getTablesInStatement()
{
    return getStrListFromValue(table);
}

QStringList SqliteExpr::getDatabasesInStatement()
{
    if (database.isNull() && !table.isNull() && validDbNames.contains(table, Qt::CaseInsensitive))
        return getStrListFromValue(table); // it's a "db.", not a "db.table."

    return getStrListFromValue(database);
}

TokenList SqliteExpr::getColumnTokensInStatement()
{
    TokenList list;
    if (!column.isNull())
    {
        if (!table.isNull())
        {
            if (!database.isNull())
                list << tokens[4];
            else
                list << tokens[2];
        }
        else
            list << tokens[0];
    }
    return list;
}

TokenList SqliteExpr::getTableTokensInStatement()
{
    TokenList list;
    if (!table.isNull())
    {
        if (!database.isNull())
            list << tokens[2];
        else
            list << tokens[0];
    }

    return list;
}

TokenList SqliteExpr::getDatabaseTokensInStatement()
{
    TokenList list;
    if (database.isNull() && !table.isNull() && validDbNames.contains(table, Qt::CaseInsensitive))
        list << tokens[0]; // it's a "db.", not a "db.table."
    else if (!database.isNull())
        list << tokens[0];

    return list;
}

QList<SqliteStatement::FullObject> SqliteExpr::getFullObjectsInStatement()
{
    QList<FullObject> result;
    if (mode != Mode::ID)
        return result;

    if (!table.isNull())
    {
        if (!database.isNull())
        {
            FullObject dbFullObject = getDbFullObject(tokens[0]);
            result << dbFullObject;
            dbTokenForFullObjects = dbFullObject.database;

            result << getFullObject(FullObject::TABLE, dbTokenForFullObjects, tokens[2]);
        }
        else
            result << getFullObject(FullObject::TABLE, dbTokenForFullObjects, tokens[0]);
    }

    return result;
}

void SqliteExpr::initId(const QString &db, const QString &table, const QString &column)
{
    mode = SqliteExpr::Mode::ID;
    database = db;
    this->table = table;
    this->column = column;
}

void SqliteExpr::initId(const QString& table, const QString& column)
{
    mode = SqliteExpr::Mode::ID;
    this->table = table;
    this->column = column;
}

void SqliteExpr::initId(const QString& column)
{
    mode = SqliteExpr::Mode::ID;
    this->column = column;
}

TokenList SqliteExpr::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;

    switch (mode)
    {
        case SqliteExpr::Mode::null:
            break;
        case SqliteExpr::Mode::LITERAL_VALUE:
        {
            if (literalNull)
                builder.withKeyword("NULL");
            else
                builder.withLiteralValue(literalValue);
            break;
        }
        case SqliteExpr::Mode::CTIME:
            builder.withKeyword(ctime.toUpper());
            break;
        case SqliteExpr::Mode::BIND_PARAM:
            builder.withBindParam(bindParam);
            break;
        case SqliteExpr::Mode::ID:
            builder.withTokens(rebuildId());
            break;
        case SqliteExpr::Mode::UNARY_OP:
            builder.withOperator(unaryOp).withSpace().withStatement(expr1);
            break;
        case SqliteExpr::Mode::BINARY_OP:
            builder.withStatement(expr1).withSpace().withOperator(binaryOp).withSpace().withStatement(expr2);
            break;
        case SqliteExpr::Mode::PTR_OP:
            builder.withStatement(expr1).withSpace().withOperator(ptrOp).withSpace().withStatement(expr2);
            break;
        case SqliteExpr::Mode::FUNCTION:
            builder.withOther(function).withParLeft();
            if (distinctKw)
                builder.withKeyword("DISTINCT");
            else if (allKw)
                builder.withKeyword("DISTINCT");

            if (star)
                builder.withOperator("*").withParRight();
            else
                builder.withStatementList(exprList).withParRight();

            break;
        case SqliteExpr::Mode::WINDOW_FUNCTION:
            builder.withOther(function).withParLeft();
            if (distinctKw)
                builder.withKeyword("DISTINCT");
            else if (allKw)
                builder.withKeyword("DISTINCT");

            if (star)
                builder.withOperator("*").withParRight();
            else
                builder.withStatementList(exprList).withParRight();

            builder.withSpace().withStatement(filterOver);
            break;
        case SqliteExpr::Mode::SUB_EXPR:
            builder.withParLeft().withStatement(expr1).withParRight();
            break;
        case SqliteExpr::Mode::CAST:
            builder.withKeyword("CAST").withSpace().withParLeft().withStatement(expr1).withSpace().withKeyword("AS")
                    .withStatement(columnType).withParRight();
            break;
        case SqliteExpr::Mode::COLLATE:
            builder.withStatement(expr1).withSpace().withKeyword("COLLATE").withSpace().withOther(collation);
            break;
        case SqliteExpr::Mode::LIKE:
            builder.withTokens(rebuildLike());
            break;
        case SqliteExpr::Mode::NULL_:
            builder.withKeyword("NULL");
            break;
        case SqliteExpr::Mode::NOTNULL:
            builder.withStatement(expr1).withSpace().withTokens(rebuildNotNull());
            break;
        case SqliteExpr::Mode::IS:
            builder.withTokens(rebuildIs());
            break;
        case SqliteExpr::Mode::DISTINCT:
            builder.withTokens(rebuildDistinct());
            break;
        case SqliteExpr::Mode::BETWEEN:
            builder.withTokens(rebuildBetween());
            break;
        case SqliteExpr::Mode::IN:
            builder.withTokens(rebuildIn());
            break;
        case SqliteExpr::Mode::ROW_VALUE:
            builder.withParLeft().withStatementList(exprList).withParRight();
            break;
        case SqliteExpr::Mode::EXISTS:
            builder.withKeyword("EXISTS").withParLeft().withStatement(select).withParRight();
            break;
        case SqliteExpr::Mode::CASE:
            builder.withTokens(rebuildCase());
            break;
        case SqliteExpr::Mode::SUB_SELECT:
            builder.withParLeft().withStatement(select).withParRight();
            break;
        case SqliteExpr::Mode::RAISE:
            builder.withStatement(raiseFunction);
            break;
    }

    return builder.build();
}

void SqliteExpr::evaluatePostParsing()
{
    detectDoubleQuotes(false); // not recursively, as SqliteStatement will take care of recursiveness
}

TokenList SqliteExpr::rebuildId()
{
    StatementTokenBuilder builder;
    if (!database.isNull())
        builder.withOther(database).withOperator(".");

    if (!table.isNull())
        builder.withOther(table).withOperator(".");

    if (table.isNull() && possibleDoubleQuotedString)
        builder.withStringPossiblyOther(column);
    else
        builder.withOther(column);

    return builder.build();
}

TokenList SqliteExpr::rebuildLike()
{
    StatementTokenBuilder builder;
    builder.withStatement(expr1).withSpace();
    if (notKw)
        builder.withKeyword("NOT").withSpace();

    builder.withKeyword(likeOp(like)).withSpace().withStatement(expr2);
    if (expr3)
        builder.withSpace().withKeyword("ESCAPE").withStatement(expr3);

    return builder.build();
}

TokenList SqliteExpr::rebuildNotNull()
{
    StatementTokenBuilder builder;
    switch (notNull)
    {
        case SqliteExpr::NotNull::ISNULL:
            builder.withKeyword("ISNULL");
            break;
        case SqliteExpr::NotNull::NOT_NULL:
            builder.withKeyword("NOT").withSpace().withKeyword("NULL");
            break;
        case SqliteExpr::NotNull::NOTNULL:
            builder.withKeyword("NOTNULL");
            break;
        case SqliteExpr::NotNull::null:
            break;
    }
    return builder.build();
}

TokenList SqliteExpr::rebuildIs()
{
    StatementTokenBuilder builder;
    builder.withStatement(expr1).withSpace().withKeyword("IS");
    if (notKw)
        builder.withSpace().withKeyword("NOT");

    builder.withStatement(expr2);
    return builder.build();
}

TokenList SqliteExpr::rebuildDistinct()
{
    StatementTokenBuilder builder;
    builder.withStatement(expr1).withSpace().withKeyword("IS");
    if (notKw)
        builder.withSpace().withKeyword("NOT");

    builder.withSpace().withKeyword("DISTINCT").withSpace().withKeyword("FROM").withSpace().withStatement(expr2);
    return builder.build();
}

TokenList SqliteExpr::rebuildBetween()
{
    StatementTokenBuilder builder;
    builder.withStatement(expr1);

    if (notKw)
        builder.withSpace().withKeyword("NOT");

    builder.withSpace().withKeyword("BETWEEN").withStatement(expr2).withSpace().withKeyword("AND").withStatement(expr3);
    return builder.build();
}

TokenList SqliteExpr::rebuildIn()
{
    StatementTokenBuilder builder;
    builder.withStatement(expr1);

    if (notKw)
        builder.withSpace().withKeyword("NOT");

    builder.withSpace().withKeyword("IN").withSpace();
    if (select)
    {
        builder.withParLeft().withStatement(select).withParRight();
    }
    else if (exprList.size() > 0)
    {
        builder.withParLeft().withStatementList(exprList).withParRight();
    }
    else
    {
        if (!database.isNull())
            builder.withOther(database).withOperator(".");

        builder.withOther(table);
    }
    return builder.build();
}

TokenList SqliteExpr::rebuildCase()
{
    StatementTokenBuilder builder;
    builder.withKeyword("CASE");
    if (expr1)
        builder.withStatement(expr1);

    builder.withSpace();

    bool then = false;
    for (SqliteExpr* expr : exprList)
    {
        if (then)
            builder.withKeyword("THEN");
        else
            builder.withKeyword("WHEN");

        builder.withStatement(expr).withSpace();
        then = !then;
    }

    if (expr2)
        builder.withKeyword("ELSE").withStatement(expr2).withSpace();

    builder.withKeyword("END");
    return builder.build();
}

void SqliteExpr::initDistinct(int distinct)
{
    if (distinct == 1)
        distinctKw = true;
    else if (distinct == 2)
        allKw = true;
}
