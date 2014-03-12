#include "sqliteinsert.h"
#include "sqlitequerytype.h"
#include "sqliteexpr.h"
#include "sqliteselect.h"
#include "parser/statementtokenbuilder.h"
#include "common/global.h"

SqliteInsert::SqliteInsert()
{
    queryType = SqliteQueryType::Insert;
}

SqliteInsert::SqliteInsert(const SqliteInsert& other) :
    SqliteQuery(other), replaceKw(other.replaceKw), defaultValuesKw(other.defaultValuesKw), onConflict(other.onConflict), database(other.database),
    table(other.table), columnNames(other.columnNames)
{
    // Special case of deep collection copy
    QList<SqliteExpr*> newExprList;
    SqliteExpr* newExpr;
    foreach (const QList<SqliteExpr*>& exprList, other.values)
    {
        newExprList.clear();
        foreach (SqliteExpr* expr, exprList)
        {
            newExpr = new SqliteExpr(*expr);
            newExpr->setParent(this);
            newExprList << newExpr;
        }
        values << newExprList;
    }

    DEEP_COPY_FIELD(SqliteSelect, select);
}

SqliteInsert::SqliteInsert(bool replace, SqliteConflictAlgo onConflict, const QString &name1, const QString &name2, const QList<QString> &columns, const QList<QList<SqliteExpr *> > &rows)
    : SqliteInsert()
{
    initName(name1, name2);
    initMode(replace, onConflict);
    columnNames = columns;
    values = rows;

    foreach (QList<SqliteExpr*> list, values)
        foreach (SqliteExpr* expr, list)
            expr->setParent(this);
}

SqliteInsert::SqliteInsert(bool replace, SqliteConflictAlgo onConflict, const QString &name1, const QString &name2, const QList<QString> &columns, const QList<SqliteExpr *> &row)
{
    initName(name1, name2);
    initMode(replace, onConflict);
    columnNames = columns;
    values += row;

    foreach (SqliteExpr* expr, row)
        expr->setParent(this);
}

SqliteInsert::SqliteInsert(bool replace, SqliteConflictAlgo onConflict, const QString &name1, const QString &name2, const QList<QString> &columns, SqliteSelect *select)
{
    initName(name1, name2);
    initMode(replace, onConflict);

    columnNames = columns;
    this->select = select;
    if (select)
        select->setParent(this);
}

SqliteInsert::SqliteInsert(bool replace, SqliteConflictAlgo onConflict, const QString &name1, const QString &name2, const QList<QString> &columns)
{
    initName(name1, name2);
    initMode(replace, onConflict);

    columnNames = columns;
    defaultValuesKw = true;
}

SqliteInsert::~SqliteInsert()
{
//    if (select)
//        delete select;

//    QList<SqliteExpr*> list;
//    SqliteExpr* expr;
//    foreach (list, values)
//        foreach (expr, list)
    //            delete expr;
}

QStringList SqliteInsert::getColumnsInStatement()
{
    QStringList columns;
    columns += columnNames;
    return columns;
}

QStringList SqliteInsert::getTablesInStatement()
{
    return getStrListFromValue(table);
}

QStringList SqliteInsert::getDatabasesInStatement()
{
    return getStrListFromValue(database);
}

TokenList SqliteInsert::getColumnTokensInStatement()
{
    TokenList list;
    foreach (TokenPtr token, getTokenListFromNamedKey("inscollist_opt", -1))
    {
        if (token->type != Token::OTHER && token->type != Token::KEYWORD)
            continue;

        list << token;
    }
    return list;
}

TokenList SqliteInsert::getTableTokensInStatement()
{
    return getObjectTokenListFromFullname();
}

TokenList SqliteInsert::getDatabaseTokensInStatement()
{
    return getDbTokenListFromFullname();
}

QList<SqliteStatement::FullObject> SqliteInsert::getFullObjectsInStatement()
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

void SqliteInsert::initName(const QString& name1, const QString& name2)
{
    if (!name2.isNull())
    {
        database = name1;
        table = name2;
    }
    else
        table = name1;
}

void SqliteInsert::initMode(bool replace, SqliteConflictAlgo onConflict)
{
    replaceKw = replace;
    onConflict = onConflict;
}

TokenList SqliteInsert::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;

    if (replaceKw)
    {
        builder.withKeyword("REPLACE").withSpace();
    }
    else
    {
        builder.withKeyword("INSERT").withSpace();
        if (onConflict != SqliteConflictAlgo::null)
            builder.withKeyword("OR").withSpace().withKeyword(sqliteConflictAlgo(onConflict)).withSpace();
    }

    builder.withKeyword("INTO").withSpace();

    if (!database.isNull())
        builder.withOther(database, dialect).withSpace();

    builder.withOther(table, dialect).withSpace();

    if (defaultValuesKw)
    {
        builder.withKeyword("DEFAULT").withSpace().withKeyword("VALUES");
    }
    else
    {
        if (columnNames.size() > 0)
            builder.withParLeft().withOtherList(columnNames, dialect).withParRight().withSpace();

        if (select)
        {
            builder.withStatement(select);
        }
        else
        {
            builder.withKeyword("VALUES").withSpace();
            bool first = true;
            foreach (const QList<SqliteExpr*>& exprList, values)
            {
                if (!first)
                    builder.withOperator(",").withSpace();

                builder.withParLeft().withStatementList(exprList).withParRight();
                first = false;
            }
        }
    }

    return builder.build();
}
