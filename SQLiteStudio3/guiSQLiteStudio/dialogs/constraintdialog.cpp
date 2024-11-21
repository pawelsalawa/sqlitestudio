#include "constraintdialog.h"
#include "ui_constraintdialog.h"
#include "iconmanager.h"
#include "constraints/constraintpanel.h"
#include <QDebug>
#include <QPushButton>

ConstraintDialog::ConstraintDialog(Mode mode, SqliteCreateTable::Constraint* constraint, SqliteCreateTable* createTable, Db* db, QWidget* parent) :
    QDialog(parent),
    ui(new Ui::ConstraintDialog),
    mode(mode),
    db(db)
{
    ui->setupUi(this);
    type = TABLE;
    constrStatement = constraint;
    this->createTable = createTable;
    init();
}

ConstraintDialog::ConstraintDialog(Mode mode, SqliteCreateTable::Column::Constraint* constraint, SqliteCreateTable::Column* column, Db* db, QWidget* parent) :
    QDialog(parent),
    ui(new Ui::ConstraintDialog),
    mode(mode),
    db(db)
{
    ui->setupUi(this);
    type = COLUMN;
    constrStatement = constraint;
    this->columnStmt = column;
    createTable = dynamic_cast<SqliteCreateTable*>(column->parent());
    init();
}

ConstraintDialog::~ConstraintDialog()
{
    delete ui;
}

SqliteStatement* ConstraintDialog::getConstraint()
{
    return constrStatement;
}

void ConstraintDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            break;
        default:
            break;
    }
}

void ConstraintDialog::init()
{
    switch (mode)
    {
        case ConstraintDialog::NEW:
            setWindowTitle(tr("New constraint", "constraint dialog"));
            ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Create", "constraint dialog"));
            break;
        case ConstraintDialog::EDIT:
            setWindowTitle(tr("Edit constraint", "dialog window"));
            ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Apply", "constraint dialog"));
            break;
    }

    connect(this, SIGNAL(accepted()), this, SLOT(storeConfiguration()));

    // Panel object
    currentPanel = createConstraintPanel();
    if (!currentPanel)
    {
        qCritical() << "The constraint panel was not constructed. Probably the constraint type was invalid.";
        return;
    }

    currentPanel->setDb(db);
    currentPanel->setCreateTableStmt(createTable.data());
    currentPanel->setColumnStmt(columnStmt.data());
    currentPanel->setConstraint(constrStatement);

    connect(currentPanel, SIGNAL(updateValidation()), this, SLOT(validate()));
    validate();

    // Put everything in place
    updateDefinitionHeader();
    ui->definitionWidget->layout()->addWidget(currentPanel);

    adjustSize();
    currentPanel->setFocus();
}

ConstraintDialog::Constraint ConstraintDialog::getSelectedConstraint()
{
    switch (type)
    {
        case ConstraintDialog::TABLE:
            return getSelectedConstraint(dynamic_cast<SqliteCreateTable::Constraint*>(constrStatement));
        case ConstraintDialog::COLUMN:
            return getSelectedConstraint(dynamic_cast<SqliteCreateTable::Column::Constraint*>(constrStatement));
    }
    return UNKNOWN;
}

ConstraintDialog::Constraint ConstraintDialog::getSelectedConstraint(SqliteCreateTable::Constraint* constraint)
{
    switch (constraint->type)
    {
        case SqliteCreateTable::Constraint::PRIMARY_KEY:
            return PK;
        case SqliteCreateTable::Constraint::UNIQUE:
            return UNIQUE;
        case SqliteCreateTable::Constraint::CHECK:
            return CHECK;
        case SqliteCreateTable::Constraint::FOREIGN_KEY:
            return FK;
        case SqliteCreateTable::Constraint::NAME_ONLY:
            break;
    }
    return UNKNOWN;
}

ConstraintDialog::Constraint ConstraintDialog::getSelectedConstraint(SqliteCreateTable::Column::Constraint* constraint)
{
    switch (constraint->type)
    {
        case SqliteCreateTable::Column::Constraint::PRIMARY_KEY:
            return PK;
        case SqliteCreateTable::Column::Constraint::NOT_NULL:
            return NOTNULL;
        case SqliteCreateTable::Column::Constraint::UNIQUE:
            return UNIQUE;
        case SqliteCreateTable::Column::Constraint::CHECK:
            return CHECK;
        case SqliteCreateTable::Column::Constraint::DEFAULT:
            return DEFAULT;
        case SqliteCreateTable::Column::Constraint::GENERATED:
            return GENERATED;
        case SqliteCreateTable::Column::Constraint::COLLATE:
            return COLLATE;
        case SqliteCreateTable::Column::Constraint::FOREIGN_KEY:
            return FK;
        case SqliteCreateTable::Column::Constraint::NULL_:
        case SqliteCreateTable::Column::Constraint::NAME_ONLY:
        case SqliteCreateTable::Column::Constraint::DEFERRABLE_ONLY:
            break;
    }
    return UNKNOWN;
}

ConstraintPanel* ConstraintDialog::createConstraintPanel()
{
    if (!constrStatement)
        return nullptr;

    if (type == COLUMN)
        return ConstraintPanel::produce(dynamic_cast<SqliteCreateTable::Column::Constraint*>(constrStatement));
    else
        return ConstraintPanel::produce(dynamic_cast<SqliteCreateTable::Constraint*>(constrStatement));
}

void ConstraintDialog::updateDefinitionHeader()
{
    switch (getSelectedConstraint())
    {
        case ConstraintDialog::UNKNOWN:
            return;
        case ConstraintDialog::PK:
            ui->titleIcon->setPixmap(ICONS.CONSTRAINT_PRIMARY_KEY);
            ui->titleLabel->setText(tr("Primary key", "table constraints"));
            break;
        case ConstraintDialog::FK:
            ui->titleIcon->setPixmap(ICONS.CONSTRAINT_FOREIGN_KEY);
            ui->titleLabel->setText(tr("Foreign key", "table constraints"));
            break;
        case ConstraintDialog::UNIQUE:
            ui->titleIcon->setPixmap(ICONS.CONSTRAINT_UNIQUE);
            ui->titleLabel->setText(tr("Unique", "table constraints"));
            break;
        case ConstraintDialog::NOTNULL:
            ui->titleIcon->setPixmap(ICONS.CONSTRAINT_NOT_NULL);
            ui->titleLabel->setText(tr("Not NULL", "table constraints"));
            break;
        case ConstraintDialog::CHECK:
            ui->titleIcon->setPixmap(ICONS.CONSTRAINT_CHECK);
            ui->titleLabel->setText(tr("Check", "table constraints"));
            break;
        case ConstraintDialog::GENERATED:
            ui->titleIcon->setPixmap(ICONS.CONSTRAINT_GENERATED);
            ui->titleLabel->setText(tr("Generated", "table constraints"));
            break;
        case ConstraintDialog::COLLATE:
            ui->titleIcon->setPixmap(ICONS.CONSTRAINT_COLLATION);
            ui->titleLabel->setText(tr("Collate", "table constraints"));
            break;
        case ConstraintDialog::DEFAULT:
            ui->titleIcon->setPixmap(ICONS.CONSTRAINT_DEFAULT);
            ui->titleLabel->setText(tr("Default", "table constraints"));
            break;
    }
}

void ConstraintDialog::validate()
{
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(currentPanel->validate());
}

void ConstraintDialog::storeConfiguration()
{
    if (!currentPanel)
    {
        qWarning() << "Called to store constraint configuration, but there's no current panel.";
        return;
    }

    currentPanel->storeDefinition();
}
