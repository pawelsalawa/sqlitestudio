#include "tableprimarykeypanel.h"
#include "ui_tablepkanduniquepanel.h"
#include <QDebug>

TablePrimaryKeyPanel::TablePrimaryKeyPanel(QWidget *parent) :
    TablePrimaryKeyAndUniquePanel(parent)
{
}

void TablePrimaryKeyPanel::storeConfiguration()
{
    TablePrimaryKeyAndUniquePanel::storeConfiguration();

    if (constraint.isNull())
        return;

    // Type
    SqliteCreateTable::Constraint* constr = dynamic_cast<SqliteCreateTable::Constraint*>(constraint.data());
    constr->type = SqliteCreateTable::Constraint::PRIMARY_KEY;

    // Autoincr
    constr->autoincrKw = ui->autoIncrCheckBox->isChecked();
}


void TablePrimaryKeyPanel::readConstraint()
{
    TablePrimaryKeyAndUniquePanel::readConstraint();

    if (constraint.isNull())
        return;

    SqliteCreateTable::Constraint* constr = dynamic_cast<SqliteCreateTable::Constraint*>(constraint.data());

    // Autoincr
    if (constr->autoincrKw)
        ui->autoIncrCheckBox->setChecked(true);
}

void TablePrimaryKeyPanel::updateState()
{
    TablePrimaryKeyAndUniquePanel::updateState();

    // Autoincr
    QStringList columns;
    QWidget* item = nullptr;
    QCheckBox* cb = nullptr;
    for (int i = 0; i < totalColumns; i++)
    {
        item = columnsLayout->itemAtPosition(i, 0)->widget();
        cb = qobject_cast<QCheckBox*>(item);
        if (cb->isChecked())
            columns << cb->text();
    }

    if (columns.size() != 1)
    {
        ui->autoIncrCheckBox->setChecked(false);
        ui->autoIncrCheckBox->setEnabled(false);
        return;
    }

    SqliteCreateTable* createTable = dynamic_cast<SqliteCreateTable*>(constraint->parentStatement());
    QString colName = columns.first();
    SqliteCreateTable::Column* column = createTable->getColumn(colName);
    if (!column)
    {
        qCritical() << "Could not find column when checking for AUTOINCREMENT checkbox state:" << colName
                    << "\nCreateTable statement:" << createTable->detokenize();
        ui->autoIncrCheckBox->setChecked(false);
        ui->autoIncrCheckBox->setEnabled(false);
        return;
    }

    if (!column->type || column->type->detokenize().trimmed().compare("INTEGER", Qt::CaseInsensitive) != 0)
    {
        ui->autoIncrCheckBox->setChecked(false);
        ui->autoIncrCheckBox->setEnabled(false);
        return;
    }

    ui->autoIncrCheckBox->setEnabled(true);
}
