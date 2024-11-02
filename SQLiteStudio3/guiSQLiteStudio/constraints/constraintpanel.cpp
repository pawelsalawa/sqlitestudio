#include "constraintpanel.h"
#include "common/unused.h"
#include "constraints/tableprimarykeypanel.h"
#include "constraints/tableforeignkeypanel.h"
#include "constraints/tableuniquepanel.h"
#include "constraints/tablecheckpanel.h"
#include "constraints/columncheckpanel.h"
#include "constraints/columncollatepanel.h"
#include "constraints/columngeneratedpanel.h"
#include "constraints/columndefaultpanel.h"
#include "constraints/columnforeignkeypanel.h"
#include "constraints/columnnotnullpanel.h"
#include "constraints/columnprimarykeypanel.h"
#include "constraints/columnuniquepanel.h"
#include <QDebug>

ConstraintPanel::ConstraintPanel(QWidget *parent) :
    QWidget(parent)
{
}

ConstraintPanel::~ConstraintPanel()
{
}

void ConstraintPanel::setConstraint(SqliteStatement* stmt)
{
    constraint = stmt;
    constraintAvailable();
}

void ConstraintPanel::setCreateTableStmt(SqliteCreateTable *stmt)
{
    createTableStmt = stmt;
}

void ConstraintPanel::setColumnStmt(SqliteCreateTable::Column *stmt)
{
    columnStmt = stmt;
}

void ConstraintPanel::storeDefinition()
{
    storeConfiguration();
    constraint->rebuildTokens();
}

void ConstraintPanel::setDb(Db* value)
{
    db = value;
}

bool ConstraintPanel::validateOnly()
{
    return validate();
}

ConstraintPanel* ConstraintPanel::produce(SqliteCreateTable::Constraint* constr)
{
    switch (constr->type)
    {
        case SqliteCreateTable::Constraint::PRIMARY_KEY:
            return new TablePrimaryKeyPanel();
        case SqliteCreateTable::Constraint::UNIQUE:
            return new TableUniquePanel();
        case SqliteCreateTable::Constraint::CHECK:
            return new TableCheckPanel();
        case SqliteCreateTable::Constraint::FOREIGN_KEY:
            return new TableForeignKeyPanel();
        case SqliteCreateTable::Constraint::NAME_ONLY:
            break;
    }

    qCritical() << "No panel defined in ConstraintPanel::createConstraintPanel()!";
    Q_ASSERT_X(true, "ConstraintPanel::produce()", "No panel defined!");
    return nullptr;
}

ConstraintPanel* ConstraintPanel::produce(SqliteCreateTable::Column::Constraint* constr)
{
    switch (constr->type)
    {
        case SqliteCreateTable::Column::Constraint::PRIMARY_KEY:
            return new ColumnPrimaryKeyPanel();
        case SqliteCreateTable::Column::Constraint::NOT_NULL:
            return new ColumnNotNullPanel();
        case SqliteCreateTable::Column::Constraint::UNIQUE:
            return new ColumnUniquePanel();
        case SqliteCreateTable::Column::Constraint::CHECK:
            return new ColumnCheckPanel();
        case SqliteCreateTable::Column::Constraint::DEFAULT:
            return new ColumnDefaultPanel();
        case SqliteCreateTable::Column::Constraint::COLLATE:
            return new ColumnCollatePanel();
        case SqliteCreateTable::Column::Constraint::GENERATED:
            return new ColumnGeneratedPanel();
        case SqliteCreateTable::Column::Constraint::FOREIGN_KEY:
            return new ColumnForeignKeyPanel();
        case SqliteCreateTable::Column::Constraint::NULL_:
        case SqliteCreateTable::Column::Constraint::NAME_ONLY:
        case SqliteCreateTable::Column::Constraint::DEFERRABLE_ONLY:
            break;
    }

    qCritical() << "No panel defined in ConstraintPanel::createConstraintPanel()!";
    Q_ASSERT_X(true, "ConstraintPanel::produce()", "No panel defined");
    return nullptr;
}

