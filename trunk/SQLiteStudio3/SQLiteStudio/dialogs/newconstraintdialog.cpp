#include "newconstraintdialog.h"
#include "ui_newconstraintdialog.h"
#include "iconmanager.h"
#include <QCommandLinkButton>

NewConstraintDialog::NewConstraintDialog(SqliteCreateTable* createTable, Db* db, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewConstraintDialog),
    type(ConstraintDialog::TABLE),
    db(db),
    createTable(createTable)
{
    ui->setupUi(this);
    init();
}

NewConstraintDialog::NewConstraintDialog(SqliteCreateTable::Column* column, Db* db, QWidget* parent) :
    QDialog(parent),
    ui(new Ui::NewConstraintDialog),
    type(ConstraintDialog::COLUMN),
    db(db),
    columnStmt(column)
{
    ui->setupUi(this);
    createTable = dynamic_cast<SqliteCreateTable*>(column->parent());
    init();
}

NewConstraintDialog::NewConstraintDialog(ConstraintDialog::Constraint constraintType, SqliteCreateTable* createTable, Db* db, QWidget* parent) :
    NewConstraintDialog(createTable, db, parent)
{
    predefinedConstraintType = constraintType;
}

NewConstraintDialog::NewConstraintDialog(ConstraintDialog::Constraint constraintType, SqliteCreateTable::Column* column, Db* db, QWidget* parent) :
    NewConstraintDialog(column, db, parent)
{
    predefinedConstraintType = constraintType;
}

NewConstraintDialog::~NewConstraintDialog()
{
    delete ui;
    if (constraintDialog)
        delete constraintDialog;
}

SqliteStatement* NewConstraintDialog::getConstraint()
{
    return constrStatement;
}

void NewConstraintDialog::changeEvent(QEvent *e)
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

void NewConstraintDialog::init()
{
    switch (type)
    {
        case ConstraintDialog::TABLE:
        {
            initTable();
            break;
        }
        case ConstraintDialog::COLUMN:
        {
            initColumn();
            break;
        }
    }
    adjustSize();
    setMaximumSize(size());
    setMinimumSize(size());
}

void NewConstraintDialog::initTable()
{
    addButton("pk", tr("Primary Key", "new constraint dialog"), SLOT(createTablePk()));
    if (createTable->dialect == Dialect::Sqlite3)
        addButton("fk", tr("Foreign Key", "new constraint dialog"), SLOT(createTableFk()));

    addButton("unique", tr("Unique", "new constraint dialog"), SLOT(createTableUnique()));
    addButton("check", tr("Check", "new constraint dialog"), SLOT(createTableCheck()));
}

void NewConstraintDialog::initColumn()
{
    addButton("pk", tr("Primary Key", "new constraint dialog"), SLOT(createColumnPk()));
    if (createTable->dialect == Dialect::Sqlite3)
        addButton("fk", tr("Foreign Key", "new constraint dialog"), SLOT(createColumnFk()));

    addButton("unique", tr("Unique", "new constraint dialog"), SLOT(createColumnUnique()));
    addButton("check", tr("Check", "new constraint dialog"), SLOT(createColumnCheck()));
    addButton("not_null", tr("Not NULL", "new constraint dialog"), SLOT(createColumnNotNull()));
    if (createTable->dialect == Dialect::Sqlite3)
        addButton("collation", tr("Collate", "new constraint dialog"), SLOT(createColumnCollate()));

    addButton("default", tr("Default", "new constraint dialog"), SLOT(createColumnDefault()));
}

void NewConstraintDialog::addButton(const QString& icon, const QString text, const char* slot)
{
    QCommandLinkButton* btn = new QCommandLinkButton();
    btn->setIcon(ICON(icon));
    btn->setText(text);
    connect(btn, SIGNAL(clicked()), this, slot);
    ui->container->layout()->addWidget(btn);
}

int NewConstraintDialog::createColumnConstraint(ConstraintDialog::Constraint constraintType)
{
    SqliteCreateTable::Column::Constraint* constraint = new SqliteCreateTable::Column::Constraint();
    switch (constraintType)
    {
        case ConstraintDialog::PK:
            constraint->type = SqliteCreateTable::Column::Constraint::PRIMARY_KEY;
            break;
        case ConstraintDialog::FK:
            constraint->type = SqliteCreateTable::Column::Constraint::FOREIGN_KEY;
            break;
        case ConstraintDialog::UNIQUE:
            constraint->type = SqliteCreateTable::Column::Constraint::UNIQUE;
            break;
        case ConstraintDialog::NOTNULL:
            constraint->type = SqliteCreateTable::Column::Constraint::NOT_NULL;
            break;
        case ConstraintDialog::CHECK:
            constraint->type = SqliteCreateTable::Column::Constraint::CHECK;
            break;
        case ConstraintDialog::COLLATE:
            constraint->type = SqliteCreateTable::Column::Constraint::COLLATE;
            break;
        case ConstraintDialog::DEFAULT:
            constraint->type = SqliteCreateTable::Column::Constraint::DEFAULT;
            break;
        case ConstraintDialog::UNKNOWN:
            break;
    }

    constrStatement = constraint;
    constrStatement->setParent(columnStmt);

    return editConstraint();
}

int NewConstraintDialog::createTableConstraint(ConstraintDialog::Constraint constraintType)
{
    SqliteCreateTable::Constraint* constraint = new SqliteCreateTable::Constraint();
    switch (constraintType)
    {
        case ConstraintDialog::PK:
            constraint->type = SqliteCreateTable::Constraint::PRIMARY_KEY;
            break;
        case ConstraintDialog::FK:
            constraint->type = SqliteCreateTable::Constraint::FOREIGN_KEY;
            break;
        case ConstraintDialog::UNIQUE:
            constraint->type = SqliteCreateTable::Constraint::UNIQUE;
            break;
        case ConstraintDialog::CHECK:
            constraint->type = SqliteCreateTable::Constraint::CHECK;
            break;
        case ConstraintDialog::NOTNULL:
        case ConstraintDialog::COLLATE:
        case ConstraintDialog::DEFAULT:
        case ConstraintDialog::UNKNOWN:
            break;
    }

    constrStatement = constraint;
    constrStatement->setParent(createTable);

    return editConstraint();
}

int NewConstraintDialog::editConstraint()
{
    switch (type)
    {
        case ConstraintDialog::TABLE:
            constraintDialog = new ConstraintDialog(ConstraintDialog::NEW, dynamic_cast<SqliteCreateTable::Constraint*>(constrStatement),
                                createTable.data(), db, parentWidget());
            break;
        case ConstraintDialog::COLUMN:
            constraintDialog = new ConstraintDialog(ConstraintDialog::NEW, dynamic_cast<SqliteCreateTable::Column::Constraint*>(constrStatement),
                                columnStmt.data(), db, parentWidget());
            break;
    }

    connect(constraintDialog, SIGNAL(rejected()), this, SLOT(reject()));
    connect(constraintDialog, SIGNAL(accepted()), this, SLOT(accept()));

    hide();
    return constraintDialog->exec();
}

void NewConstraintDialog::createTablePk()
{
    createTableConstraint(ConstraintDialog::PK);
}

void NewConstraintDialog::createTableFk()
{
    createTableConstraint(ConstraintDialog::FK);
}

void NewConstraintDialog::createTableUnique()
{
    createTableConstraint(ConstraintDialog::UNIQUE);
}

void NewConstraintDialog::createTableCheck()
{
    createTableConstraint(ConstraintDialog::CHECK);
}

void NewConstraintDialog::createColumnPk()
{
    createColumnConstraint(ConstraintDialog::PK);
}

void NewConstraintDialog::createColumnFk()
{
    createColumnConstraint(ConstraintDialog::FK);
}

void NewConstraintDialog::createColumnUnique()
{
    createColumnConstraint(ConstraintDialog::UNIQUE);
}

void NewConstraintDialog::createColumnCheck()
{
    createColumnConstraint(ConstraintDialog::CHECK);
}

void NewConstraintDialog::createColumnNotNull()
{
    createColumnConstraint(ConstraintDialog::NOTNULL);
}

void NewConstraintDialog::createColumnDefault()
{
    createColumnConstraint(ConstraintDialog::DEFAULT);
}

void NewConstraintDialog::createColumnCollate()
{
    createColumnConstraint(ConstraintDialog::COLLATE);
}

int NewConstraintDialog::exec()
{
    if (predefinedConstraintType == ConstraintDialog::UNKNOWN)
        return QDialog::exec();

    switch (type)
    {
        case ConstraintDialog::TABLE:
            return createTableConstraint(predefinedConstraintType);
        case ConstraintDialog::COLUMN:
            return createColumnConstraint(predefinedConstraintType);
    }

    return QDialog::Rejected;
}
