#include "columndialog.h"
#include "ui_columndialog.h"
#include "columndialogconstraintsmodel.h"
#include "iconmanager.h"
#include "newconstraintdialog.h"
#include "dialogs/constraintdialog.h"
#include "constraints/constraintpanel.h"
#include "datatype.h"
#include "uiutils.h"
#include <QDebug>
#include <QCheckBox>
#include <QMessageBox>
#include <QDebug>
#include <QPushButton>

ColumnDialog::ColumnDialog(Db* db, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ColumnDialog),
    db(db)
{
    init();
}

ColumnDialog::~ColumnDialog()
{
    delete ui;
}

void ColumnDialog::init()
{
    ui->setupUi(this);
    setWindowIcon(ICONS.COLUMN);

    ui->scale->setStrict(true);
    ui->precision->setStrict(true);

    ui->typeCombo->addItem("");
    foreach (DataType::Enum type, DataType::getAllTypes())
        ui->typeCombo->addItem(DataType::toString(type));

    connect(ui->typeCombo, SIGNAL(currentTextChanged(QString)), this, SLOT(updateDataType()));

    constraintsModel = new ColumnDialogConstraintsModel();
    ui->constraintsView->setModel(constraintsModel);
    initActions();

    setupConstraintCheckBoxes();

    connect(ui->advancedCheck, SIGNAL(toggled(bool)), this, SLOT(switchMode(bool)));

    connect(ui->constraintsView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(updateConstraintsToolbarState()));
    connect(ui->constraintsView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(editConstraint(QModelIndex)));
    connect(constraintsModel, SIGNAL(constraintsChanged()), this, SLOT(updateConstraints()));
    connect(constraintsModel, SIGNAL(constraintsChanged()), this, SLOT(updateState()));

    connect(ui->pkButton, SIGNAL(clicked()), this, SLOT(configurePk()));
    connect(ui->fkButton, SIGNAL(clicked()), this, SLOT(configureFk()));
    connect(ui->checkButton, SIGNAL(clicked()), this, SLOT(configureCheck()));
    connect(ui->defaultButton, SIGNAL(clicked()), this, SLOT(configureDefault()));
    connect(ui->notNullButton, SIGNAL(clicked()), this, SLOT(configureNotNull()));
    connect(ui->collateButton, SIGNAL(clicked()), this, SLOT(configureCollate()));
    connect(ui->uniqueButton, SIGNAL(clicked()), this, SLOT(configureUnique()));

    updateState();
}

void ColumnDialog::changeEvent(QEvent *e)
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

void ColumnDialog::createActions()
{
    createAction(ADD_CONSTRAINT, ICONS.COLUMN_CONSTRAINT_ADD, tr("Add constraint", "column dialog"), this, SLOT(addConstraint()), ui->constraintsToolbar);
    createAction(EDIT_CONSTRAINT, ICONS.COLUMN_CONSTRAINT_EDIT, tr("Edit constraint", "column dialog"), this, SLOT(editConstraint()), ui->constraintsToolbar);
    createAction(DEL_CONSTRAINT, ICONS.COLUMN_CONSTRAINT_DEL, tr("Delete constraint", "column dialog"), this, SLOT(delConstraint()), ui->constraintsToolbar);
    createAction(MOVE_CONSTRAINT_UP, ICONS.MOVE_UP, tr("Move constraint up", "column dialog"), this, SLOT(moveConstraintUp()), ui->constraintsToolbar);
    createAction(MOVE_CONSTRAINT_DOWN, ICONS.MOVE_DOWN, tr("Move constraint down", "column dialog"), this, SLOT(moveConstraintDown()), ui->constraintsToolbar);
    ui->constraintsToolbar->addSeparator();
    createAction(ADD_PK, ICONS.CONSTRAINT_PRIMARY_KEY_ADD, tr("Add a primary key", "column dialog"), this, SLOT(addPk()), ui->constraintsToolbar);
    createAction(ADD_FK, ICONS.CONSTRAINT_FOREIGN_KEY_ADD, tr("Add a foreign key", "column dialog"), this, SLOT(addFk()), ui->constraintsToolbar);
    createAction(ADD_UNIQUE, ICONS.CONSTRAINT_UNIQUE_ADD, tr("Add an unique constraint", "column dialog"), this, SLOT(addUnique()), ui->constraintsToolbar);
    createAction(ADD_CHECK, ICONS.CONSTRAINT_CHECK_ADD, tr("Add a check constraint", "column dialog"), this, SLOT(addCheck()), ui->constraintsToolbar);
    createAction(ADD_NOT_NULL, ICONS.CONSTRAINT_NOT_NULL_ADD, tr("Add a not null constraint", "column dialog"), this, SLOT(addNotNull()), ui->constraintsToolbar);
    createAction(ADD_COLLATE, ICONS.CONSTRAINT_COLLATION_ADD, tr("Add a collate constraint", "column dialog"), this, SLOT(addCollate()), ui->constraintsToolbar);
    createAction(ADD_DEFAULT, ICONS.CONSTRAINT_DEFAULT_ADD, tr("Add a default constraint", "column dialog"), this, SLOT(addDefault()), ui->constraintsToolbar);
}

void ColumnDialog::setupDefShortcuts()
{
}

void ColumnDialog::updateConstraintsToolbarState()
{
    QModelIndex idx = ui->constraintsView->selectionModel()->currentIndex();
    bool hasSelected = idx.isValid();
    bool isFirst = false;
    bool isLast = false;
    if (constraintsModel->rowCount() > 0)
    {
        isFirst = (idx.row() == 0);
        isLast = (idx.row() == (constraintsModel->rowCount() - 1));
    }

    actionMap[EDIT_CONSTRAINT]->setEnabled(hasSelected);
    actionMap[DEL_CONSTRAINT]->setEnabled(hasSelected);
    actionMap[MOVE_CONSTRAINT_UP]->setEnabled(hasSelected && !isFirst);
    actionMap[MOVE_CONSTRAINT_DOWN]->setEnabled(hasSelected && !isLast);
}

void ColumnDialog::updateState()
{
    ui->pkButton->setEnabled(ui->pkCheck->isChecked());
    ui->fkButton->setEnabled(ui->fkCheck->isChecked());
    ui->uniqueButton->setEnabled(ui->uniqueCheck->isChecked());
    ui->notNullButton->setEnabled(ui->notNullCheck->isChecked());
    ui->checkButton->setEnabled(ui->checkCheck->isChecked());
    ui->collateButton->setEnabled(ui->collateCheck->isChecked());
    ui->defaultButton->setEnabled(ui->defaultCheck->isChecked());
    updateConstraintsToolbarState();
}

void ColumnDialog::addConstraint(ConstraintDialog::Constraint mode)
{
    NewConstraintDialog dialog(mode, column.data(), db, this);
    if (dialog.exec() != QDialog::Accepted)
        return;

    SqliteStatement* constrStmt = dialog.getConstraint();
    SqliteCreateTable::Column::Constraint* constr = dynamic_cast<SqliteCreateTable::Column::Constraint*>(constrStmt);
    if (!constr)
    {
        qCritical() << "Constraint returned from ConstraintDialog was not of column type, while we're trying to add column constraint.";
        return;
    }

    constraintsModel->appendConstraint(constr);
    ui->constraintsView->resizeColumnToContents(0);
    ui->constraintsView->resizeColumnToContents(1);
}

void ColumnDialog::setupConstraintCheckBoxes()
{
    ui->pkCheck->setIcon(ICONS.CONSTRAINT_PRIMARY_KEY);
    ui->fkCheck->setIcon(ICONS.CONSTRAINT_FOREIGN_KEY);
    ui->uniqueCheck->setIcon(ICONS.CONSTRAINT_UNIQUE);
    ui->notNullCheck->setIcon(ICONS.CONSTRAINT_NOT_NULL);
    ui->checkCheck->setIcon(ICONS.CONSTRAINT_CHECK);
    ui->collateCheck->setIcon(ICONS.CONSTRAINT_COLLATION);
    ui->defaultCheck->setIcon(ICONS.CONSTRAINT_DEFAULT);

    connect(ui->pkCheck, SIGNAL(clicked(bool)), this, SLOT(pkToggled(bool)));
    connect(ui->fkCheck, SIGNAL(clicked(bool)), this, SLOT(fkToggled(bool)));
    connect(ui->uniqueCheck, SIGNAL(clicked(bool)), this, SLOT(uniqueToggled(bool)));
    connect(ui->notNullCheck, SIGNAL(clicked(bool)), this, SLOT(notNullToggled(bool)));
    connect(ui->checkCheck, SIGNAL(clicked(bool)), this, SLOT(checkToggled(bool)));
    connect(ui->collateCheck, SIGNAL(clicked(bool)), this, SLOT(collateToggled(bool)));
    connect(ui->defaultCheck, SIGNAL(clicked(bool)), this, SLOT(defaultToggled(bool)));

    for (QCheckBox* cb : {
         ui->pkCheck,
         ui->fkCheck,
         ui->uniqueCheck,
         ui->notNullCheck,
         ui->checkCheck,
         ui->collateCheck,
         ui->defaultCheck
    })
    {
        connect(cb, SIGNAL(toggled(bool)), this, SLOT(updateState()));
    }
}

void ColumnDialog::addConstraint()
{
    addConstraint(ConstraintDialog::UNKNOWN);
}

void ColumnDialog::editConstraint()
{
    QModelIndex idx = ui->constraintsView->currentIndex();
    editConstraint(idx);
}

void ColumnDialog::delConstraint()
{
    QModelIndex idx = ui->constraintsView->currentIndex();
    delConstraint(idx);
}

void ColumnDialog::editConstraint(const QModelIndex& idx)
{
    if (!idx.isValid())
        return;

    SqliteCreateTable::Column::Constraint* constr = constraintsModel->getConstraint(idx.row());
    editConstraint(constr);
}

void ColumnDialog::editConstraint(SqliteCreateTable::Column::Constraint* constraint)
{
    ConstraintDialog dialog(ConstraintDialog::EDIT, constraint, column.data(), db, this);
    if (dialog.exec() != QDialog::Accepted)
        return;

    ui->constraintsView->resizeColumnToContents(0);
    ui->constraintsView->resizeColumnToContents(1);
    updateConstraints();
}

void ColumnDialog::delConstraint(const QModelIndex& idx)
{
    if (!idx.isValid())
        return;

    SqliteCreateTable::Column::Constraint* constr = constraintsModel->getConstraint(idx.row());

    QString arg = constr->name.isNull() ? constr->typeString() : constr->name;
    QString msg = tr("Are you sure you want to delete constraint '%1'?", "column dialog").arg(arg);
    int btn = QMessageBox::question(this, tr("Delete constraint", "column dialog"), msg);
    if (btn != QMessageBox::Yes)
        return;

    constraintsModel->delConstraint(idx.row());
}

void ColumnDialog::configureConstraint(SqliteCreateTable::Column::Constraint::Type type)
{
    SqliteCreateTable::Column::Constraint* constraint = column->getConstraint(type);
    if (!constraint)
    {
        qCritical() << "Called ColumnDialog::configureConstraint(), but there's no specified type constraint in the column!";
        return;
    }
    editConstraint(constraint);
}

void ColumnDialog::addEmptyConstraint(SqliteCreateTable::Column::Constraint::Type type)
{
    SqliteCreateTable::Column::Constraint* constr = new SqliteCreateTable::Column::Constraint();
    constr->type = type;
    constraintsModel->appendConstraint(constr);
    constr->rebuildTokens();
}

void ColumnDialog::delAllConstraint(SqliteCreateTable::Column::Constraint::Type type)
{
    SqliteCreateTable::Column::Constraint* constr;
    while ((constr = column->getConstraint(type)) != nullptr)
        constraintsModel->delConstraint(constr);
}

void ColumnDialog::constraintToggled(SqliteCreateTable::Column::Constraint::Type type, bool enabled)
{
    if (enabled)
        addEmptyConstraint(type);
    else
        delAllConstraint(type);
}

void ColumnDialog::updateConstraintState(SqliteCreateTable::Column::Constraint* constraint)
{
    QToolButton* toolButton = getToolButtonForConstraint(constraint);
    if (!toolButton)
        return;

    bool result = true;
    ConstraintPanel* panel = ConstraintPanel::produce(constraint);
    if (!panel)
    {
        qCritical() << "Could not produce ConstraintPanel for constraint validation in ColumnDialog::updateConstraintState().";
    }
    else
    {
        panel->setDb(db);
        panel->setConstraint(constraint);
        result = panel->validateOnly();
        delete panel;
    }

    QString errMsg = tr("Correct the constraint's configuration.");
    if (db->getDialect() == Dialect::Sqlite2 && isUnofficialSqlite2Constraint(constraint))
    {
        QString tooltip = tr("This constraint is not officially supported by SQLite 2,\nbut it's okay to use it.");
        setValidStateWihtTooltip(toolButton, tooltip, result, errMsg);
    }
    else
    {
        setValidState(toolButton, result, errMsg);
    }

    if (!result)
    {
        QPushButton* btn = ui->buttonBox->button(QDialogButtonBox::Ok);
        btn->setEnabled(false);
    }
}

QCheckBox* ColumnDialog::getCheckBoxForConstraint(SqliteCreateTable::Column::Constraint* constraint)
{
    switch (constraint->type)
    {
        case SqliteCreateTable::Column::Constraint::PRIMARY_KEY:
            return ui->pkCheck;
        case SqliteCreateTable::Column::Constraint::NOT_NULL:
            return ui->notNullCheck;
        case SqliteCreateTable::Column::Constraint::UNIQUE:
            return ui->uniqueCheck;
        case SqliteCreateTable::Column::Constraint::CHECK:
            return ui->checkCheck;
        case SqliteCreateTable::Column::Constraint::DEFAULT:
            return ui->defaultCheck;
        case SqliteCreateTable::Column::Constraint::COLLATE:
            return ui->collateCheck;
        case SqliteCreateTable::Column::Constraint::FOREIGN_KEY:
            return ui->fkCheck;
        case SqliteCreateTable::Column::Constraint::NULL_:
        case SqliteCreateTable::Column::Constraint::NAME_ONLY:
        case SqliteCreateTable::Column::Constraint::DEFERRABLE_ONLY:
            break;
    }
    return nullptr;
}

QToolButton* ColumnDialog::getToolButtonForConstraint(SqliteCreateTable::Column::Constraint* constraint)
{
    switch (constraint->type)
    {
        case SqliteCreateTable::Column::Constraint::PRIMARY_KEY:
            return ui->pkButton;
        case SqliteCreateTable::Column::Constraint::NOT_NULL:
            return ui->notNullButton;
        case SqliteCreateTable::Column::Constraint::UNIQUE:
            return ui->uniqueButton;
        case SqliteCreateTable::Column::Constraint::CHECK:
            return ui->checkButton;
        case SqliteCreateTable::Column::Constraint::DEFAULT:
            return ui->defaultButton;
        case SqliteCreateTable::Column::Constraint::COLLATE:
            return ui->collateButton;
        case SqliteCreateTable::Column::Constraint::FOREIGN_KEY:
            return ui->fkButton;
        case SqliteCreateTable::Column::Constraint::NULL_:
        case SqliteCreateTable::Column::Constraint::NAME_ONLY:
        case SqliteCreateTable::Column::Constraint::DEFERRABLE_ONLY:
            break;
    }
    return nullptr;
}

bool ColumnDialog::isUnofficialSqlite2Constraint(SqliteCreateTable::Column::Constraint* constraint)
{
    switch (constraint->type)
    {
        case SqliteCreateTable::Column::Constraint::FOREIGN_KEY:
        case SqliteCreateTable::Column::Constraint::COLLATE:
            return true;
        case SqliteCreateTable::Column::Constraint::PRIMARY_KEY:
        case SqliteCreateTable::Column::Constraint::NOT_NULL:
        case SqliteCreateTable::Column::Constraint::UNIQUE:
        case SqliteCreateTable::Column::Constraint::CHECK:
        case SqliteCreateTable::Column::Constraint::DEFAULT:
        case SqliteCreateTable::Column::Constraint::NULL_:
        case SqliteCreateTable::Column::Constraint::NAME_ONLY:
        case SqliteCreateTable::Column::Constraint::DEFERRABLE_ONLY:
            break;
    }
    return false;
}

void ColumnDialog::moveConstraintUp()
{
    QModelIndex idx = ui->constraintsView->currentIndex();
    if (!idx.isValid())
        return;

    constraintsModel->moveConstraintUp(idx.row());
}

void ColumnDialog::moveConstraintDown()
{
    QModelIndex idx = ui->constraintsView->currentIndex();
    if (!idx.isValid())
        return;

    constraintsModel->moveConstraintDown(idx.row());
}

void ColumnDialog::addPk()
{
    addConstraint(ConstraintDialog::PK);
}

void ColumnDialog::addFk()
{
    addConstraint(ConstraintDialog::FK);
}

void ColumnDialog::addUnique()
{
    addConstraint(ConstraintDialog::UNIQUE);
}

void ColumnDialog::addCheck()
{
    addConstraint(ConstraintDialog::CHECK);
}

void ColumnDialog::addCollate()
{
    addConstraint(ConstraintDialog::COLLATE);
}

void ColumnDialog::addNotNull()
{
    addConstraint(ConstraintDialog::NOTNULL);
}

void ColumnDialog::addDefault()
{
    addConstraint(ConstraintDialog::DEFAULT);
}

void ColumnDialog::configurePk()
{
    configureConstraint(SqliteCreateTable::Column::Constraint::PRIMARY_KEY);
}

void ColumnDialog::configureFk()
{
    configureConstraint(SqliteCreateTable::Column::Constraint::FOREIGN_KEY);
}

void ColumnDialog::configureUnique()
{
    configureConstraint(SqliteCreateTable::Column::Constraint::UNIQUE);
}

void ColumnDialog::configureCheck()
{
    configureConstraint(SqliteCreateTable::Column::Constraint::CHECK);
}

void ColumnDialog::configureCollate()
{
    configureConstraint(SqliteCreateTable::Column::Constraint::COLLATE);
}

void ColumnDialog::configureNotNull()
{
    configureConstraint(SqliteCreateTable::Column::Constraint::NOT_NULL);
}

void ColumnDialog::configureDefault()
{
    configureConstraint(SqliteCreateTable::Column::Constraint::DEFAULT);
}

void ColumnDialog::pkToggled(bool enabled)
{
    constraintToggled(SqliteCreateTable::Column::Constraint::PRIMARY_KEY, enabled);
}

void ColumnDialog::fkToggled(bool enabled)
{
    constraintToggled(SqliteCreateTable::Column::Constraint::FOREIGN_KEY, enabled);
}

void ColumnDialog::uniqueToggled(bool enabled)
{
    constraintToggled(SqliteCreateTable::Column::Constraint::UNIQUE, enabled);
}

void ColumnDialog::checkToggled(bool enabled)
{
    constraintToggled(SqliteCreateTable::Column::Constraint::CHECK, enabled);
}

void ColumnDialog::collateToggled(bool enabled)
{
    constraintToggled(SqliteCreateTable::Column::Constraint::COLLATE, enabled);
}

void ColumnDialog::notNullToggled(bool enabled)
{
    constraintToggled(SqliteCreateTable::Column::Constraint::NOT_NULL, enabled);
}

void ColumnDialog::defaultToggled(bool enabled)
{
    constraintToggled(SqliteCreateTable::Column::Constraint::DEFAULT, enabled);
}

void ColumnDialog::switchMode(bool advanced)
{
    ui->constraintModesWidget->setCurrentWidget(advanced ? ui->advancedPage : ui->simplePage);
}

void ColumnDialog::updateConstraints()
{
    QPushButton* btn = ui->buttonBox->button(QDialogButtonBox::Ok);
    btn->setEnabled(true);

    for (QCheckBox* cb : {
         ui->pkCheck,
         ui->fkCheck,
         ui->uniqueCheck,
         ui->notNullCheck,
         ui->checkCheck,
         ui->collateCheck,
         ui->defaultCheck
    })
    {
        cb->setChecked(false);
    }

    for (QToolButton* tb : {
         ui->pkButton,
         ui->fkButton,
         ui->uniqueButton,
         ui->notNullButton,
         ui->checkButton,
         ui->collateButton,
         ui->defaultButton
    })
    {
        setValidState(tb, true);
    }

    foreach (SqliteCreateTable::Column::Constraint* constr, column->constraints)
        updateConstraint(constr);
}

void ColumnDialog::updateConstraint(SqliteCreateTable::Column::Constraint* constraint)
{
    QCheckBox* checkBox = getCheckBoxForConstraint(constraint);
    if (checkBox)
    {
        checkBox->setChecked(true);
        updateConstraintState(constraint);
    }
}

void ColumnDialog::setColumn(SqliteCreateTable::Column* value)
{
    column = SqliteCreateTable::ColumnPtr::create(*value);
    column->setParent(value->parent());
    constraintsModel->setColumn(column.data());

    ui->name->setText(value->name);
    if (value->type)
    {
        ui->typeCombo->setEditText(value->type->name);
        ui->scale->setValue(value->type->scale, false);
        ui->precision->setValue(value->type->precision, false);
    }

    updateConstraints();
}

SqliteCreateTable::Column* ColumnDialog::getModifiedColumn()
{
    column->name = ui->name->text();
    updateDataType();
    column->rebuildTokens();

    return new SqliteCreateTable::Column(*column);
}

void ColumnDialog::updateDataType()
{
    if (!column)
        return;

    QString typeTxt = ui->typeCombo->currentText();
    QString scaleTxt = ui->scale->getValue().toString();
    QString precisionTxt = ui->precision->getValue().toString();
    if (!typeTxt.isEmpty())
    {
        if (!column->type)
        {
            column->type = new SqliteColumnType();
            column->type->setParent(column.data());
        }

        column->type->name = typeTxt;

        if (!scaleTxt.isEmpty())
            column->type->scale = ui->scale->getValue();

        if (!precisionTxt.isEmpty())
            column->type->precision = ui->precision->getValue();

        column->type->rebuildTokens();
    }
    else if (column->type) // there was a type, but there's not now
    {
        delete column->type;
        column->type = nullptr;
    }
}
