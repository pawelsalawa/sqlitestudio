#include "columnuniquepanel.h"
#include "parser/ast/sqlitecreatetable.h"

ColumnUniquePanel::ColumnUniquePanel(QWidget *parent) :
    ColumnUniqueAndNotNullPanel(parent)
{
}

void ColumnUniquePanel::storeType()
{
    SqliteCreateTable::Column::Constraint* constr = dynamic_cast<SqliteCreateTable::Column::Constraint*>(constraint.data());
    constr->type = SqliteCreateTable::Column::Constraint::UNIQUE;
}
