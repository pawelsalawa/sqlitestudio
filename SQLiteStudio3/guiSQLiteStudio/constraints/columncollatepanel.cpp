#include "columncollatepanel.h"
#include "ui_columncollatepanel.h"
#include "parser/ast/sqlitecreatetable.h"
#include "schemaresolver.h"
#include "uiutils.h"
#include <QStringListModel>

ColumnCollatePanel::ColumnCollatePanel(QWidget *parent) :
    ConstraintPanel(parent),
    ui(new Ui::ColumnCollatePanel)
{
    ui->setupUi(this);
    init();
}

ColumnCollatePanel::~ColumnCollatePanel()
{
    delete ui;
}

void ColumnCollatePanel::changeEvent(QEvent *e)
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

void ColumnCollatePanel::init()
{
    collationModel = new QStringListModel(this);
    ui->collationCombo->setModel(collationModel);
    connect(ui->namedCheck, SIGNAL(toggled(bool)), this, SIGNAL(updateValidation()));
    connect(ui->namedEdit, SIGNAL(textChanged(QString)), this, SIGNAL(updateValidation()));
    connect(ui->collationCombo->lineEdit(), SIGNAL(textChanged(QString)), this, SIGNAL(updateValidation()));
    connect(ui->namedCheck, SIGNAL(toggled(bool)), this, SLOT(updateState()));
    updateState();
}

void ColumnCollatePanel::readConstraint()
{
    SqliteCreateTable::Column::Constraint* constr = dynamic_cast<SqliteCreateTable::Column::Constraint*>(constraint.data());
    ui->collationCombo->setCurrentText(constr->collationName);

    if (!constr->name.isNull())
    {
        ui->namedCheck->setChecked(true);
        ui->namedEdit->setText(constr->name);
    }
}

void ColumnCollatePanel::readCollations()
{
    SchemaResolver resolver(db);
    QStringList collList = resolver.getCollations();

    if (collList.size() > 0)
        collList.prepend("");

    collationModel->setStringList(collList);
}

void ColumnCollatePanel::updateState()
{
    ui->namedEdit->setEnabled(ui->namedCheck->isChecked());
}

bool ColumnCollatePanel::validate()
{
    bool nameOk = true;
    if (ui->namedCheck->isChecked() && ui->namedEdit->text().isEmpty())
        nameOk = false;

    bool collationOk = !ui->collationCombo->currentText().isEmpty();

    setValidState(ui->namedEdit, nameOk, tr("Enter a name of the constraint."));
    setValidState(ui->collationCombo, collationOk, tr("Enter a collation name."));

    return nameOk && collationOk;
}

void ColumnCollatePanel::constraintAvailable()
{
    if (constraint.isNull())
        return;

    readCollations();
    readConstraint();
}

void ColumnCollatePanel::storeConfiguration()
{
    if (constraint.isNull())
        return;

    SqliteCreateTable::Column::Constraint* constr = dynamic_cast<SqliteCreateTable::Column::Constraint*>(constraint.data());
    constr->type = SqliteCreateTable::Column::Constraint::COLLATE;

    if (ui->namedCheck->isChecked())
        constr->name = ui->namedEdit->text();
    else
        constr->name.clear();

    constr->collationName = ui->collationCombo->currentText();
}
