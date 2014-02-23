#include "columnforeignkeypanel.h"
#include "ui_columnforeignkeypanel.h"
#include "schemaresolver.h"
#include "uiutils.h"
#include <QDebug>
#include <QSignalMapper>

ColumnForeignKeyPanel::ColumnForeignKeyPanel(QWidget *parent) :
    ConstraintPanel(parent),
    ui(new Ui::ColumnForeignKeyPanel)
{
    ui->setupUi(this);
    init();
}

ColumnForeignKeyPanel::~ColumnForeignKeyPanel()
{
    delete ui;
}

void ColumnForeignKeyPanel::changeEvent(QEvent *e)
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


bool ColumnForeignKeyPanel::validate()
{
    bool tableOk = (ui->fkTableCombo->currentIndex() > -1);
    bool columnOk = (ui->fkColumnCombo->currentIndex() > -1);
    bool nameOk = !ui->namedCheckBox->isChecked() || !ui->nameEdit->text().isEmpty();

    setValidStyle(ui->fkTableLabel, tableOk);
    setValidStyle(ui->fkColumnLabel, columnOk);
    setValidStyle(ui->namedCheckBox, nameOk);

    return tableOk && columnOk && nameOk;
}

void ColumnForeignKeyPanel::constraintAvailable()
{
    readTables();
    readConstraint();
}

void ColumnForeignKeyPanel::init()
{
    setFocusProxy(ui->fkTableCombo);

    ui->fkColumnCombo->setModel(&fkColumnsModel);
    connect(ui->fkColumnCombo, SIGNAL(currentIndexChanged(int)), this, SIGNAL(updateValidation()));

    connect(ui->namedCheckBox, SIGNAL(toggled(bool)), this, SIGNAL(updateValidation()));
    connect(ui->nameEdit, SIGNAL(textChanged(QString)), this, SIGNAL(updateValidation()));
    connect(ui->fkTableCombo, SIGNAL(currentIndexChanged(int)), this, SIGNAL(updateValidation()));
    connect(ui->fkTableCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(updateFkColumns()));
    connect(ui->fkTableCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(updateState()));
    connect(ui->onDeleteCheckBox, SIGNAL(toggled(bool)), this, SLOT(updateState()));
    connect(ui->onUpdateCheckBox, SIGNAL(toggled(bool)), this, SLOT(updateState()));
    connect(ui->matchCheckBox, SIGNAL(toggled(bool)), this, SLOT(updateState()));

    ui->deferrableCombo->addItems({
                                      "",
                                      sqliteDeferrable(SqliteDeferrable::DEFERRABLE),
                                      sqliteDeferrable(SqliteDeferrable::NOT_DEFERRABLE)
                                  });
    ui->initiallyCombo->addItems({
                                     "",
                                     sqliteInitially(SqliteInitially::DEFERRED),
                                     sqliteInitially(SqliteInitially::IMMEDIATE),
                                 });

    QStringList reactions = {
        SqliteForeignKey::Condition::toString(SqliteForeignKey::Condition::NO_ACTION),
        SqliteForeignKey::Condition::toString(SqliteForeignKey::Condition::SET_NULL),
        SqliteForeignKey::Condition::toString(SqliteForeignKey::Condition::SET_DEFAULT),
        SqliteForeignKey::Condition::toString(SqliteForeignKey::Condition::CASCADE),
        SqliteForeignKey::Condition::toString(SqliteForeignKey::Condition::RESTRICT)
    };
    ui->onUpdateCombo->addItems(reactions);
    ui->onDeleteCombo->addItems(reactions);
    ui->matchCombo->addItems({"SIMPLE", "FULL", "PARTIAL"});

    connect(ui->namedCheckBox, SIGNAL(toggled(bool)), this, SLOT(updateState()));
    updateState();
}

void ColumnForeignKeyPanel::updateState()
{
    bool tableSelected = (ui->fkTableCombo->currentIndex() > -1);
    ui->fkColumnCombo->setEnabled(tableSelected);
    ui->deferrableCombo->setEnabled(tableSelected);
    ui->initiallyCombo->setEnabled(tableSelected);
    ui->namedCheckBox->setEnabled(tableSelected);
    ui->nameEdit->setEnabled(tableSelected && ui->namedCheckBox->isChecked());
    ui->onDeleteCheckBox->setEnabled(tableSelected);
    ui->onUpdateCheckBox->setEnabled(tableSelected);
    ui->matchCheckBox->setEnabled(tableSelected);
    ui->onDeleteCombo->setEnabled(tableSelected && ui->onDeleteCheckBox->isChecked());
    ui->onUpdateCombo->setEnabled(tableSelected && ui->onUpdateCheckBox->isChecked());
    ui->matchCombo->setEnabled(tableSelected && ui->matchCheckBox->isChecked());
}

void ColumnForeignKeyPanel::updateFkColumns()
{
    QStringList columns;
    if (ui->fkTableCombo->currentIndex() == -1)
    {
        fkColumnsModel.setStringList(columns);
        updateState();
        return;
    }

    SchemaResolver resolver(db);
    columns = resolver.getTableColumns(ui->fkTableCombo->currentText()); // TODO named db attach not supported
    fkColumnsModel.setStringList(columns);
}

void ColumnForeignKeyPanel::readConstraint()
{
    if (constraint.isNull())
        return;

    SqliteCreateTable::Column* column = dynamic_cast<SqliteCreateTable::Column*>(constraint->parent());
    SqliteCreateTable::Column::Constraint* constr = dynamic_cast<SqliteCreateTable::Column::Constraint*>(constraint.data());
    if (!constr->foreignKey)
        return;

    // Table
    if (!constr->foreignKey->foreignTable.isNull())
        ui->fkTableCombo->setCurrentText(constr->foreignKey->foreignTable);

    // Conditions
    foreach (SqliteForeignKey::Condition* condition, constr->foreignKey->conditions)
        readCondition(condition);

    // Initially, Deferrable
    ui->deferrableCombo->setCurrentText(sqliteDeferrable(constr->foreignKey->deferrable));
    ui->initiallyCombo->setCurrentText(sqliteInitially(constr->foreignKey->initially));

    // Name
    if (!constr->name.isNull())
    {
        ui->namedCheckBox->setChecked(true);
        ui->nameEdit->setText(constr->name);
    }

    // Column
    if (constr->foreignKey->indexedColumns.size() > 1)
    {
        qWarning() << "More than one referenced column in the column foreign key:" << constr->detokenize();
        return;
    }

    QString fkColumn = column->name;
    if (constr->foreignKey->indexedColumns.size() == 1)
        fkColumn = constr->foreignKey->indexedColumns.first()->name;

    ui->fkColumnCombo->setCurrentText(fkColumn);
}

void ColumnForeignKeyPanel::readCondition(SqliteForeignKey::Condition* condition)
{
    switch (condition->action)
    {
        case SqliteForeignKey::Condition::UPDATE:
            ui->onUpdateCheckBox->setChecked(true);
            ui->onUpdateCombo->setCurrentText(SqliteForeignKey::Condition::toString(condition->reaction));
            break;
        case SqliteForeignKey::Condition::INSERT:
            // INSERT is not officially supported.
            break;
        case SqliteForeignKey::Condition::DELETE:
            ui->onDeleteCheckBox->setChecked(true);
            ui->onDeleteCombo->setCurrentText(SqliteForeignKey::Condition::toString(condition->reaction));
            break;
        case SqliteForeignKey::Condition::MATCH:
            ui->matchCheckBox->setChecked(true);
            ui->matchCombo->setCurrentText(SqliteForeignKey::Condition::toString(condition->reaction));
            break;
    }
}

void ColumnForeignKeyPanel::storeConfiguration()
{
    if (constraint.isNull())
        return;

    // Type
    SqliteCreateTable::Column::Constraint* constr = dynamic_cast<SqliteCreateTable::Column::Constraint*>(constraint.data());
    constr->type = SqliteCreateTable::Column::Constraint::FOREIGN_KEY;

    // Cleanup & initial setup
    if (constr->foreignKey)
        delete constr->foreignKey;

    constr->foreignKey = new SqliteForeignKey();
    constr->foreignKey->setParent(constr);

    // Foreign table
    constr->foreignKey->foreignTable = ui->fkTableCombo->currentText();

    // Column
    SqliteIndexedColumn* idxCol = new SqliteIndexedColumn(ui->fkColumnCombo->currentText());
    idxCol->setParent(constr->foreignKey);
    constr->foreignKey->indexedColumns << idxCol;

    // Actions/reactions
    if (ui->onDeleteCheckBox->isChecked())
        storeCondition(SqliteForeignKey::Condition::DELETE, ui->onDeleteCombo->currentText());

    if (ui->onUpdateCheckBox->isChecked())
        storeCondition(SqliteForeignKey::Condition::UPDATE, ui->onDeleteCombo->currentText());

    if (ui->matchCheckBox->isChecked())
        storeMatchCondition(ui->matchCombo->currentText());

    // Deferred/initially
    constr->foreignKey->deferrable = sqliteDeferrable(ui->deferrableCombo->currentText());
    constr->foreignKey->initially = sqliteInitially(ui->initiallyCombo->currentText());

    // Name
    constr->name = QString::null;
    if (ui->namedCheckBox->isChecked())
        constr->name = ui->nameEdit->text();
}

void ColumnForeignKeyPanel::storeCondition(SqliteForeignKey::Condition::Action action, const QString& reaction)
{
    SqliteCreateTable::Column::Constraint* constr = dynamic_cast<SqliteCreateTable::Column::Constraint*>(constraint.data());

    SqliteForeignKey::Condition* condition = new SqliteForeignKey::Condition(
                action,
                SqliteForeignKey::Condition::toEnum(reaction)
            );
    condition->setParent(constr->foreignKey);
    constr->foreignKey->conditions << condition;
}

void ColumnForeignKeyPanel::storeMatchCondition(const QString& reaction)
{
    SqliteCreateTable::Column::Constraint* constr = dynamic_cast<SqliteCreateTable::Column::Constraint*>(constraint.data());

    SqliteForeignKey::Condition* condition = new SqliteForeignKey::Condition(reaction);
    condition->setParent(constr->foreignKey);
    constr->foreignKey->conditions << condition;
}

void ColumnForeignKeyPanel::readTables()
{
    SchemaResolver resolver(db);
    resolver.setIgnoreSystemObjects(true);
    QStringList tables = resolver.getTables(); // TODO named db attach not supported

    tables.sort(Qt::CaseInsensitive);

    ui->fkTableCombo->addItems(tables);
    ui->fkTableCombo->setCurrentIndex(-1);
}
