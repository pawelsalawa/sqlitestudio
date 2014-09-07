#include "tablecheckpanel.h"
#include "parser/ast/sqlitecreatetable.h"
#include "parser/parser.h"
#include <QDebug>

TableCheckPanel::TableCheckPanel(QWidget *parent) :
    ConstraintCheckPanel(parent)
{
}

SqliteExpr* TableCheckPanel::readExpr()
{
    SqliteCreateTable::Constraint* constr = dynamic_cast<SqliteCreateTable::Constraint*>(constraint.data());
    return constr->expr;
}

QString TableCheckPanel::readName()
{
    SqliteCreateTable::Constraint* constr = dynamic_cast<SqliteCreateTable::Constraint*>(constraint.data());
    return constr->name;
}

void TableCheckPanel::storeType()
{
    SqliteCreateTable::Constraint* constr = dynamic_cast<SqliteCreateTable::Constraint*>(constraint.data());
    constr->type = SqliteCreateTable::Constraint::CHECK;
}

SqliteConflictAlgo TableCheckPanel::readConflictAlgo()
{
    SqliteCreateTable::Constraint* constr = dynamic_cast<SqliteCreateTable::Constraint*>(constraint.data());
    return constr->onConflict;
}

void TableCheckPanel::storeExpr(SqliteExpr* expr)
{
    SqliteCreateTable::Constraint* constr = dynamic_cast<SqliteCreateTable::Constraint*>(constraint.data());
    constr->expr = expr;
}

void TableCheckPanel::storeName(const QString& name)
{
    SqliteCreateTable::Constraint* constr = dynamic_cast<SqliteCreateTable::Constraint*>(constraint.data());
    constr->name = name;
}

void TableCheckPanel::storeConflictAlgo(SqliteConflictAlgo algo)
{
    SqliteCreateTable::Constraint* constr = dynamic_cast<SqliteCreateTable::Constraint*>(constraint.data());
    constr->onConflict = algo;
}

SqliteCreateTable* TableCheckPanel::getCreateTable()
{
    return dynamic_cast<SqliteCreateTable*>(constraint->parentStatement());
}
