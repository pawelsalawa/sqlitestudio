#include "tableuniquepanel.h"
#include "ui_tablepkanduniquepanel.h"

TableUniquePanel::TableUniquePanel(QWidget *parent) :
    TablePrimaryKeyAndUniquePanel(parent)
{
    ui->autoIncrCheckBox->setVisible(false);
}


void TableUniquePanel::storeConfiguration()
{
    TablePrimaryKeyAndUniquePanel::storeConfiguration();

    if (constraint.isNull())
        return;

    // Type
    SqliteCreateTable::Constraint* constr = dynamic_cast<SqliteCreateTable::Constraint*>(constraint.data());
    constr->type = SqliteCreateTable::Constraint::UNIQUE;
}
