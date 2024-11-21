#include "tableforeignkeypanel.h"
#include "ui_tableforeignkeypanel.h"
#include "schemaresolver.h"
#include "uiutils.h"
#include "common/widgetstateindicator.h"
#include <QDebug>
#include <QSignalMapper>

TableForeignKeyPanel::TableForeignKeyPanel(QWidget *parent) :
    ConstraintPanel(parent),
    ui(new Ui::TableForeignKeyPanel)
{
    ui->setupUi(this);
    init();
}

TableForeignKeyPanel::~TableForeignKeyPanel()
{
    delete ui;
}

void TableForeignKeyPanel::changeEvent(QEvent *e)
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


bool TableForeignKeyPanel::validate()
{
    bool tableOk = (ui->fkTableCombo->currentIndex() > -1);

    bool columnsOk = false;
    bool columnsSelected = true;
    bool idxOk = true;
    QCheckBox* check = nullptr;
    QComboBox* combo = nullptr;
    for (int i = 0; i < totalColumns; i++)
    {
        check = qobject_cast<QCheckBox*>(columnsLayout->itemAtPosition(i, 0)->widget());
        combo = qobject_cast<QComboBox*>(columnsLayout->itemAtPosition(i, 1)->widget());
        if (check->isChecked())
        {
            columnsOk = true;

            idxOk = (combo->currentIndex() > -1);
            setValidState(combo, idxOk, tr("Pick the foreign column."));
            if (!idxOk)
                columnsSelected = false;
            else
                handleFkTypeMatched(combo, check->property(UI_PROP_COLUMN).toString(), combo->currentText());

            break;
        }
    }

    bool nameOk = true;
    if (ui->namedCheckBox->isChecked() && ui->nameEdit->text().isEmpty())
        nameOk = false;

    setValidState(ui->fkTableCombo, tableOk, tr("Pick the foreign table."));
    setValidState(ui->columnsGroup, columnsOk, tr("Select at least one foreign column."));
    setValidState(ui->nameEdit, nameOk, tr("Enter a name of the constraint."));

    return tableOk && columnsOk && nameOk && columnsSelected;
}

void TableForeignKeyPanel::setDb(Db* value)
{
    ConstraintPanel::setDb(value);
}

void TableForeignKeyPanel::constraintAvailable()
{
    readTables();
    buildColumns();
    readConstraint();
}

void TableForeignKeyPanel::init()
{
    setFocusProxy(ui->fkTableCombo);
    columnsLayout = new QGridLayout();
    ui->columnsScrollContents->setLayout(columnsLayout);

    columnSignalMapping = new QSignalMapper(this);
    connect(columnSignalMapping, SIGNAL(mapped(int)), this, SLOT(updateColumnState(int)));

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

void TableForeignKeyPanel::updateState()
{
    bool tableSelected = (ui->fkTableCombo->currentIndex() > -1);
    for (int i = 0; i < totalColumns; i++)
        updateColumnState(i, tableSelected);

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

void TableForeignKeyPanel::updateColumnState(int rowIdx, bool tableSelected)
{
    QCheckBox* check = qobject_cast<QCheckBox*>(columnsLayout->itemAtPosition(rowIdx, 0)->widget());
    bool wasEnabled = check->isEnabled();
    check->setEnabled(tableSelected);

    QComboBox* combo = qobject_cast<QComboBox*>(columnsLayout->itemAtPosition(rowIdx, 1)->widget());
    combo->setEnabled(tableSelected && check->isChecked());

    if (!wasEnabled && check->isEnabled())
    {
        // Automatically set matching column
        int idx = fkColumnsModel.stringList().indexOf(check->property(UI_PROP_COLUMN).toString());
        if (idx > -1)
            combo->setCurrentIndex(idx);
    }
}

void TableForeignKeyPanel::updateColumnState(int rowIdx)
{
    bool tableSelected = (ui->fkTableCombo->currentIndex() > -1);
    updateColumnState(rowIdx, tableSelected);
}

void TableForeignKeyPanel::updateFkColumns()
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

void TableForeignKeyPanel::buildColumns()
{
    totalColumns = 0;
    if (constraint.isNull())
        return;

    SqliteCreateTable* createTable = dynamic_cast<SqliteCreateTable*>(constraint->parentStatement());
    int row = 0;
    for (SqliteCreateTable::Column* column : createTable->columns)
        buildColumn(column, row++);
}

void TableForeignKeyPanel::buildColumn(SqliteCreateTable::Column* column, int row)
{
    int col = 0;

    QCheckBox* check = new QCheckBox(column->name);
    check->setProperty(UI_PROP_COLUMN, column->name);
    columnsLayout->addWidget(check, row, col++);
    columnSignalMapping->setMapping(check, row);
    connect(check, SIGNAL(toggled(bool)), columnSignalMapping, SLOT(map()));
    connect(check, SIGNAL(toggled(bool)), this, SIGNAL(updateValidation()));

    QComboBox* fkCol = new QComboBox();
    fkCol->setToolTip(tr("Foreign column", "table constraints"));
    fkCol->setModel(&fkColumnsModel);
    columnsLayout->addWidget(fkCol, row, col++);
    connect(fkCol, SIGNAL(currentIndexChanged(int)), this, SIGNAL(updateValidation()));

    totalColumns++;

    updateColumnState(row);
}

void TableForeignKeyPanel::readConstraint()
{
    if (constraint.isNull())
        return;

    SqliteCreateTable::Constraint* constr = dynamic_cast<SqliteCreateTable::Constraint*>(constraint.data());
    if (!constr->foreignKey)
        return;

    if (!constr->foreignKey->foreignTable.isNull())
        ui->fkTableCombo->setCurrentText(constr->foreignKey->foreignTable);

    for (SqliteForeignKey::Condition* condition : constr->foreignKey->conditions)
        readCondition(condition);

    ui->deferrableCombo->setCurrentText(sqliteDeferrable(constr->foreignKey->deferrable));
    ui->initiallyCombo->setCurrentText(sqliteInitially(constr->foreignKey->initially));

    // Name
    if (!constr->name.isNull())
    {
        ui->namedCheckBox->setChecked(true);
        ui->nameEdit->setText(constr->name);
    }

    // Columns
    int idx;
    QCheckBox* check = nullptr;
    QComboBox* combo = nullptr;
    SqliteIndexedColumn* foreignCol = nullptr;
    int i = 0;
    for (SqliteIndexedColumn* localCol : constr->indexedColumns)
    {
        // Foreign col
        if (i < constr->foreignKey->indexedColumns.size())
            foreignCol = constr->foreignKey->indexedColumns[i];
        else
            foreignCol = nullptr;

        i++;

        // Column index
        idx = getColumnIndex(localCol->name);
        if (idx < 0)
            continue;

        // Column states
        check = dynamic_cast<QCheckBox*>(columnsLayout->itemAtPosition(idx, 0)->widget());
        check->setChecked(true);

        combo = dynamic_cast<QComboBox*>(columnsLayout->itemAtPosition(idx, 1)->widget());
        if (foreignCol)
            combo->setCurrentText(foreignCol->name);
        else if (fkColumnsModel.stringList().contains(localCol->name))
            combo->setCurrentText(localCol->name);
        else
            combo->setCurrentIndex(-1);
    }
}

void TableForeignKeyPanel::readCondition(SqliteForeignKey::Condition* condition)
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

void TableForeignKeyPanel::storeConfiguration()
{
    if (constraint.isNull())
        return;

    // Type
    SqliteCreateTable::Constraint* constr = dynamic_cast<SqliteCreateTable::Constraint*>(constraint.data());
    constr->type = SqliteCreateTable::Constraint::FOREIGN_KEY;

    // Cleanup & initial setup
    if (constr->foreignKey)
        delete constr->foreignKey;

    for (SqliteIndexedColumn* idxCol : constr->indexedColumns)
        delete idxCol;

    constr->indexedColumns.clear();

    constr->foreignKey = new SqliteForeignKey();
    constr->foreignKey->setParent(constr);

    // Foreign table
    constr->foreignKey->foreignTable = ui->fkTableCombo->currentText();

    // Columns
    QCheckBox* check = nullptr;
    QComboBox* combo = nullptr;
    SqliteIndexedColumn* idxCol = nullptr;
    for (int i = 0; i < totalColumns; i++)
    {
        // Local column
        check = dynamic_cast<QCheckBox*>(columnsLayout->itemAtPosition(i, 0)->widget());
        if (!check->isChecked())
            continue;

        idxCol = new SqliteIndexedColumn(check->property(UI_PROP_COLUMN).toString());
        idxCol->setParent(constr);
        constr->indexedColumns << idxCol;

        // Foreign column
        combo = dynamic_cast<QComboBox*>(columnsLayout->itemAtPosition(i, 1)->widget());

        idxCol = new SqliteIndexedColumn(combo->currentText());
        idxCol->setParent(constr->foreignKey);
        constr->foreignKey->indexedColumns << idxCol;
    }

    // Actions/reactions
    if (ui->onDeleteCheckBox->isChecked())
        storeCondition(SqliteForeignKey::Condition::DELETE, ui->onDeleteCombo->currentText());

    if (ui->onUpdateCheckBox->isChecked())
        storeCondition(SqliteForeignKey::Condition::UPDATE, ui->onUpdateCombo->currentText());

    if (ui->matchCheckBox->isChecked())
        storeMatchCondition(ui->matchCombo->currentText());

    // Deferred/initially
    constr->foreignKey->deferrable = sqliteDeferrable(ui->deferrableCombo->currentText());
    constr->foreignKey->initially = sqliteInitially(ui->initiallyCombo->currentText());

    // Name
    constr->name = QString();
    if (ui->namedCheckBox->isChecked())
        constr->name = ui->nameEdit->text();
}

void TableForeignKeyPanel::storeCondition(SqliteForeignKey::Condition::Action action, const QString& reaction)
{
    SqliteCreateTable::Constraint* constr = dynamic_cast<SqliteCreateTable::Constraint*>(constraint.data());

    SqliteForeignKey::Condition* condition = new SqliteForeignKey::Condition(
                action,
                SqliteForeignKey::Condition::toEnum(reaction)
            );
    condition->setParent(constr->foreignKey);
    constr->foreignKey->conditions << condition;
}

void TableForeignKeyPanel::storeMatchCondition(const QString& reaction)
{
    SqliteCreateTable::Constraint* constr = dynamic_cast<SqliteCreateTable::Constraint*>(constraint.data());

    SqliteForeignKey::Condition* condition = new SqliteForeignKey::Condition(reaction);
    condition->setParent(constr->foreignKey);
    constr->foreignKey->conditions << condition;
}

void TableForeignKeyPanel::handleFkTypeMatched(QWidget* indicatorParent, const QString &localColumn, const QString fkColumn)
{
    SqliteCreateTable::Column* column = createTableStmt->getColumn(localColumn);
    if (!column || !column->type)
        return;

    QString localType = column->type->toDataType().toString();

    // FK column type
    QString fkTable = ui->fkTableCombo->currentText();
    if (!fkTableTypesCache.contains(fkTable, Qt::CaseInsensitive))
    {
        SchemaResolver resolver(db);
        fkTableTypesCache[fkTable] = resolver.getTableColumnDataTypesByName(fkTable);
    }

    StrHash<DataType> fkTypes = fkTableTypesCache.value(fkTable, Qt::CaseInsensitive);
    if (!fkTypes.contains(fkColumn, Qt::CaseInsensitive))
        return;

    QString fkType = fkTypes.value(fkColumn, Qt::CaseInsensitive).toString();

    if (localType.toLower().trimmed() != fkType.toLower().trimmed())
    {
        setValidStateWarning(indicatorParent,
            tr("Referenced column type (%1) is different than type declared for local column (%2). It may cause issues while inserting or updating data.")
                                 .arg(fkType, localType));
    }
}

void TableForeignKeyPanel::readTables()
{
    SchemaResolver resolver(db);
    resolver.setIgnoreSystemObjects(true);
    QStringList tables = resolver.getTables(); // TODO named db attach not supported

    tables.sort(Qt::CaseInsensitive);

    ui->fkTableCombo->addItems(tables);
    ui->fkTableCombo->setCurrentIndex(-1);
}

int TableForeignKeyPanel::getColumnIndex(const QString& colName)
{
    QWidget* item = nullptr;
    QCheckBox* cb = nullptr;
    for (int i = 0; i < totalColumns; i++)
    {
        item = columnsLayout->itemAtPosition(i, 0)->widget();
        cb = qobject_cast<QCheckBox*>(item);
        if (cb->property(UI_PROP_COLUMN).toString().compare(colName, Qt::CaseInsensitive) == 0)
            return i;
    }
    return -1;
}
