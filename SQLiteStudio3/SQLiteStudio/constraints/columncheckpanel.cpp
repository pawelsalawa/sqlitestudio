#include "columncheckpanel.h"
#include "parser/ast/sqlitecreatetable.h"
#include "parser/parser.h"
#include <QDebug>

ColumnCheckPanel::ColumnCheckPanel(QWidget *parent) :
    ConstraintCheckPanel(parent)
{
}

SqliteExpr* ColumnCheckPanel::readExpr()
{
    SqliteCreateTable::Column::Constraint* constr = dynamic_cast<SqliteCreateTable::Column::Constraint*>(constraint.data());
    return constr->expr;
}

QString ColumnCheckPanel::readName()
{
    SqliteCreateTable::Column::Constraint* constr = dynamic_cast<SqliteCreateTable::Column::Constraint*>(constraint.data());
    return constr->name;
}

void ColumnCheckPanel::storeType()
{
    SqliteCreateTable::Column::Constraint* constr = dynamic_cast<SqliteCreateTable::Column::Constraint*>(constraint.data());
    constr->type = SqliteCreateTable::Column::Constraint::CHECK;
}

SqliteConflictAlgo ColumnCheckPanel::readConflictAlgo()
{
    SqliteCreateTable::Column::Constraint* constr = dynamic_cast<SqliteCreateTable::Column::Constraint*>(constraint.data());
    return constr->onConflict;
}

void ColumnCheckPanel::storeExpr(SqliteExpr* expr)
{
    SqliteCreateTable::Column::Constraint* constr = dynamic_cast<SqliteCreateTable::Column::Constraint*>(constraint.data());
    constr->expr = expr;
}

void ColumnCheckPanel::storeName(const QString& name)
{
    SqliteCreateTable::Column::Constraint* constr = dynamic_cast<SqliteCreateTable::Column::Constraint*>(constraint.data());
    constr->name = name;
}

void ColumnCheckPanel::storeConflictAlgo(SqliteConflictAlgo algo)
{
    SqliteCreateTable::Column::Constraint* constr = dynamic_cast<SqliteCreateTable::Column::Constraint*>(constraint.data());
    constr->onConflict = algo;
}

SqliteCreateTable* ColumnCheckPanel::getCreateTable()
{
    return dynamic_cast<SqliteCreateTable*>(constraint->parentStatement()->parentStatement());
}
