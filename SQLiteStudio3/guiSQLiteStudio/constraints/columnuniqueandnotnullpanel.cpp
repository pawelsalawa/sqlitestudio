#include "columnuniqueandnotnullpanel.h"
#include "ui_columnuniqueandnotnullpanel.h"
#include "parser/ast/sqlitecreatetable.h"
#include "parser/keywords.h"
#include "uiutils.h"

ColumnUniqueAndNotNullPanel::ColumnUniqueAndNotNullPanel(QWidget *parent) :
    ConstraintPanel(parent),
    ui(new Ui::ColumnUniqueAndNotNullPanel)
{
    ui->setupUi(this);
    init();
}

ColumnUniqueAndNotNullPanel::~ColumnUniqueAndNotNullPanel()
{
    delete ui;
}

void ColumnUniqueAndNotNullPanel::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            break;
        default:
            break;
    }
}

void ColumnUniqueAndNotNullPanel::init()
{
    ui->conflictCombo->addItems(getConflictAlgorithms());

    connect(ui->namedCheck, SIGNAL(toggled(bool)), this, SIGNAL(updateValidation()));
    connect(ui->namedEdit, SIGNAL(textChanged(QString)), this, SIGNAL(updateValidation()));
    connect(ui->namedCheck, SIGNAL(toggled(bool)), this, SLOT(updateState()));
    connect(ui->conflictCheck, SIGNAL(toggled(bool)), this, SLOT(updateState()));
    updateState();
}

void ColumnUniqueAndNotNullPanel::readConstraint()
{
    SqliteCreateTable::Column::Constraint* constr = dynamic_cast<SqliteCreateTable::Column::Constraint*>(constraint.data());

    if (!constr->name.isNull())
    {
        ui->namedCheck->setChecked(true);
        ui->namedEdit->setText(constr->name);
    }

    if (constr->onConflict != SqliteConflictAlgo::null)
    {
        ui->conflictCheck->setChecked(true);
        ui->conflictCombo->setCurrentText(sqliteConflictAlgo(constr->onConflict));
    }
}

void ColumnUniqueAndNotNullPanel::updateState()
{
    ui->namedEdit->setEnabled(ui->namedCheck->isChecked());
    ui->conflictCombo->setEnabled(ui->conflictCheck->isChecked());
}


bool ColumnUniqueAndNotNullPanel::validate()
{
    bool nameOk = true;
    if (ui->namedCheck->isChecked() && ui->namedEdit->text().isEmpty())
        nameOk = false;

    setValidState(ui->namedEdit, nameOk, tr("Enter a name of the constraint."));

    return nameOk;
}

void ColumnUniqueAndNotNullPanel::constraintAvailable()
{
    if (constraint.isNull())
        return;

    readConstraint();
}

void ColumnUniqueAndNotNullPanel::storeConfiguration()
{
    if (constraint.isNull())
        return;

    storeType();

    SqliteCreateTable::Column::Constraint* constr = dynamic_cast<SqliteCreateTable::Column::Constraint*>(constraint.data());
    if (ui->namedCheck->isChecked())
        constr->name = ui->namedEdit->text();
    else
        constr->name.clear();

    if (ui->conflictCheck->isChecked() && ui->conflictCombo->currentIndex() > -1)
        constr->onConflict = sqliteConflictAlgo(ui->conflictCombo->currentText());
}
