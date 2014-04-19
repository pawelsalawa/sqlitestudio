#include "columnprimarykeypanel.h"
#include "ui_columnprimarykeypanel.h"
#include "parser/ast/sqlitecreatetable.h"
#include "parser/keywords.h"
#include "datatype.h"
#include "uiutils.h"

ColumnPrimaryKeyPanel::ColumnPrimaryKeyPanel(QWidget *parent) :
    ConstraintPanel(parent),
    ui(new Ui::ColumnPrimaryKeyPanel)
{
    ui->setupUi(this);
    init();
}

ColumnPrimaryKeyPanel::~ColumnPrimaryKeyPanel()
{
    delete ui;
}

void ColumnPrimaryKeyPanel::changeEvent(QEvent *e)
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

void ColumnPrimaryKeyPanel::init()
{
    QStringList sortOrders = {sqliteSortOrder(SqliteSortOrder::ASC), sqliteSortOrder(SqliteSortOrder::DESC)};
    ui->sortOrderCombo->addItems(sortOrders);

    ui->conflictCombo->addItems(getConflictAlgorithms());

    connect(ui->namedCheck, SIGNAL(toggled(bool)), this, SIGNAL(updateValidation()));
    connect(ui->namedEdit, SIGNAL(textChanged(QString)), this, SIGNAL(updateValidation()));
    connect(ui->sortOrderCheck, SIGNAL(toggled(bool)), this, SLOT(updateState()));
    connect(ui->namedCheck, SIGNAL(toggled(bool)), this, SLOT(updateState()));
    connect(ui->conflictCheck, SIGNAL(toggled(bool)), this, SLOT(updateState()));
    updateState();
}

void ColumnPrimaryKeyPanel::readConstraint()
{
    SqliteCreateTable::Column::Constraint* constr = dynamic_cast<SqliteCreateTable::Column::Constraint*>(constraint.data());
    if (constraint->dialect == Dialect::Sqlite3)
        ui->autoIncrCheck->setChecked(constr->autoincrKw);

    if (constr->sortOrder != SqliteSortOrder::null)
    {
        ui->sortOrderCheck->setChecked(true);
        ui->sortOrderCombo->setCurrentText(sqliteSortOrder(constr->sortOrder));
    }

    if (!constr->name.isNull())
    {
        ui->namedCheck->setEnabled(true);
        ui->namedEdit->setText(constr->name);
    }

    if (constr->onConflict != SqliteConflictAlgo::null)
    {
        ui->conflictCheck->setChecked(true);
        ui->conflictCombo->setCurrentText(sqliteConflictAlgo(constr->onConflict));
    }
}

void ColumnPrimaryKeyPanel::updateState()
{
    ui->sortOrderCombo->setEnabled(ui->sortOrderCheck->isChecked());
    ui->namedEdit->setEnabled(ui->namedCheck->isChecked());
    ui->conflictCombo->setEnabled(ui->conflictCheck->isChecked());
}


bool ColumnPrimaryKeyPanel::validate()
{
    bool nameOk = true;
    if (ui->namedCheck->isChecked() && ui->namedEdit->text().isEmpty())
        nameOk = false;

    setValidState(ui->namedEdit, nameOk, tr("Enter a name of the constraint."));

    return nameOk;
}

void ColumnPrimaryKeyPanel::constraintAvailable()
{
    if (constraint.isNull())
        return;

    SqliteCreateTable::Column* column = dynamic_cast<SqliteCreateTable::Column*>(constraint->parent());
    ui->autoIncrCheck->setVisible(constraint->dialect == Dialect::Sqlite3);
    ui->autoIncrCheck->setEnabled(column->type &&
        DataType::fromString(column->type->detokenize().trimmed(), Qt::CaseInsensitive) == DataType::INTEGER);

    if (!ui->autoIncrCheck->isEnabled())
        ui->autoIncrCheck->setText(tr("Autoincrement (only for %1 type columns)", "column primary key").arg("INTEGER"));

    readConstraint();
}

void ColumnPrimaryKeyPanel::storeConfiguration()
{
    if (constraint.isNull())
        return;

    SqliteCreateTable::Column::Constraint* constr = dynamic_cast<SqliteCreateTable::Column::Constraint*>(constraint.data());
    constr->type = SqliteCreateTable::Column::Constraint::PRIMARY_KEY;

    if (constraint->dialect == Dialect::Sqlite3)
        constr->autoincrKw = ui->autoIncrCheck->isChecked();

    if (ui->sortOrderCheck->isChecked() && ui->sortOrderCombo->currentIndex() > -1)
        constr->sortOrder = sqliteSortOrder(ui->sortOrderCombo->currentText());

    if (ui->namedCheck->isChecked())
        constr->name = ui->namedEdit->text();

    if (ui->conflictCheck->isChecked() && ui->conflictCombo->currentIndex() > -1)
        constr->onConflict = sqliteConflictAlgo(ui->conflictCombo->currentText());
}
