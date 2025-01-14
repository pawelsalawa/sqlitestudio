#include "tablepkanduniquepanel.h"
#include "ui_tablepkanduniquepanel.h"
#include "parser/keywords.h"
#include "schemaresolver.h"
#include "uiutils.h"
#include <QGridLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QSignalMapper>
#include <QDebug>
#include <QScrollBar>

TablePrimaryKeyAndUniquePanel::TablePrimaryKeyAndUniquePanel(QWidget *parent) :
    ConstraintPanel(parent),
    ui(new Ui::TablePrimaryKeyAndUniquePanel)
{
    ui->setupUi(this);
    init();
}

TablePrimaryKeyAndUniquePanel::~TablePrimaryKeyAndUniquePanel()
{
    delete ui;
}

void TablePrimaryKeyAndUniquePanel::changeEvent(QEvent *e)
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

void TablePrimaryKeyAndUniquePanel::init()
{
    columnsLayout = new QGridLayout();
    ui->scrollAreaWidgetContents->setLayout(columnsLayout);

    connect(ui->namedCheckBox, SIGNAL(toggled(bool)), this, SIGNAL(updateValidation()));
    connect(ui->namedLineEdit, SIGNAL(textChanged(QString)), this, SIGNAL(updateValidation()));

    ui->conflictComboBox->addItems(getConflictAlgorithms());

    columnSignalMapping = new QSignalMapper(this);
    connect(columnSignalMapping, SIGNAL(mappedInt(int)), this, SLOT(updateColumnState(int)));

    connect(ui->namedCheckBox, SIGNAL(toggled(bool)), this, SLOT(updateState()));
    connect(ui->conflictCheckBox, SIGNAL(toggled(bool)), this, SLOT(updateState()));
    updateState();
}

void TablePrimaryKeyAndUniquePanel::readCollations()
{
    SchemaResolver resolver(db);
    QStringList collList = resolver.getCollations();

    if (collList.size() > 0)
        collList.prepend("");

    collations.setStringList(collList);
}

void TablePrimaryKeyAndUniquePanel::buildColumn(SqliteCreateTable::Column* column, int row)
{
    int col = 0;

    QCheckBox* check = new QCheckBox(column->name);
    check->setProperty(UI_PROP_COLUMN, column->name);
    columnsLayout->addWidget(check, row, col++);
    columnSignalMapping->setMapping(check, row);
    connect(check, SIGNAL(toggled(bool)), columnSignalMapping, SLOT(map()));
    connect(check, SIGNAL(toggled(bool)), this, SIGNAL(updateValidation()));

    QComboBox* collation = nullptr;
    if (!constraint.isNull())
    {
        collation = new QComboBox();
        collation->setMaximumWidth(ui->colHdrCollation->width());
        collation->setMinimumWidth(ui->colHdrCollation->width() - ui->scrollArea->verticalScrollBar()->width());
        collation->setEditable(true);
        collation->lineEdit()->setPlaceholderText(tr("Collate", "table constraints"));
        collation->setModel(&collations);
        columnsLayout->addWidget(collation, row, col++);
    }

    QComboBox* sortOrder = new QComboBox();
    sortOrder->setFixedWidth(ui->colHdrSort->width());
    sortOrder->setToolTip(tr("Sort order", "table constraints"));
    columnsLayout->addWidget(sortOrder, row, col++);

    QStringList sortList = {"", sqliteSortOrder(SqliteSortOrder::ASC), sqliteSortOrder(SqliteSortOrder::DESC)};
    sortOrder->addItems(sortList);

    totalColumns++;

    updateColumnState(row);
}

int TablePrimaryKeyAndUniquePanel::getColumnIndex(const QString& colName)
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

void TablePrimaryKeyAndUniquePanel::updateColumnState(int colIdx)
{
    QWidget* item = columnsLayout->itemAtPosition(colIdx, 0)->widget();
    QCheckBox* cb = qobject_cast<QCheckBox*>(item);
    bool enable = cb->isChecked();

    item = columnsLayout->itemAtPosition(colIdx, 1)->widget();
    qobject_cast<QComboBox*>(item)->setEnabled(enable);

    if (!constraint.isNull())
    {
        item = columnsLayout->itemAtPosition(colIdx, 2)->widget();
        qobject_cast<QComboBox*>(item)->setEnabled(enable);
    }

    updateState();
}

void TablePrimaryKeyAndUniquePanel::updateState()
{
    ui->namedLineEdit->setEnabled(ui->namedCheckBox->isChecked());
    ui->conflictComboBox->setEnabled(ui->conflictCheckBox->isChecked());
}

void TablePrimaryKeyAndUniquePanel::constraintAvailable()
{
    readCollations();
    buildColumns();
    readConstraint();
}

bool TablePrimaryKeyAndUniquePanel::validate()
{
    bool countOk  = false;
    QWidget* item = nullptr;
    QCheckBox* cb = nullptr;
    for (int i = 0; i < totalColumns; i++)
    {
        item = columnsLayout->itemAtPosition(i, 0)->widget();
        cb = qobject_cast<QCheckBox*>(item);
        if (cb->isChecked())
        {
            countOk = true;
            break;
        }
    }

    bool nameOk = true;
    if (ui->namedCheckBox->isChecked() && ui->namedLineEdit->text().isEmpty())
        nameOk = false;

    setValidState(ui->groupBox, countOk, tr("Select at least one column."));
    setValidState(ui->namedLineEdit, nameOk, tr("Enter a name of the constraint."));

    return countOk && nameOk;
}


void TablePrimaryKeyAndUniquePanel::storeConfiguration()
{
    if (constraint.isNull())
        return;

    SqliteCreateTable::Constraint* constr = dynamic_cast<SqliteCreateTable::Constraint*>(constraint.data());

    // Name
    constr->name = QString();
    if (ui->namedCheckBox->isChecked())
        constr->name = ui->namedLineEdit->text();

    // On conflict
    if (ui->conflictCheckBox->isChecked())
        constr->onConflict = sqliteConflictAlgo(ui->conflictComboBox->currentText());

    // Columns
    for (SqliteIndexedColumn* idxCol : constr->indexedColumns)
        delete idxCol;

    constr->indexedColumns.clear();

    QCheckBox* check = nullptr;
    QComboBox* combo = nullptr;
    SqliteIndexedColumn* idxCol = nullptr;
    QString name;
    QString collate;
    SqliteSortOrder sortOrder;
    for (int i = 0; i < totalColumns; i++)
    {
        check = dynamic_cast<QCheckBox*>(columnsLayout->itemAtPosition(i, 0)->widget());
        if (!check->isChecked())
            continue;

        name = check->property(UI_PROP_COLUMN).toString();

        combo = dynamic_cast<QComboBox*>(columnsLayout->itemAtPosition(i, 1)->widget());
        collate = combo->currentText();
        if (collate.isEmpty())
            collate = QString();

        combo = dynamic_cast<QComboBox*>(columnsLayout->itemAtPosition(i, 2)->widget());
        sortOrder = sqliteSortOrder(combo->currentText());

        idxCol = new SqliteIndexedColumn(name, collate, sortOrder);
        idxCol->setParent(constr);
        constr->indexedColumns << idxCol;
    }
}

void TablePrimaryKeyAndUniquePanel::readConstraint()
{
    if (constraint.isNull())
        return;

    SqliteCreateTable::Constraint* constr = dynamic_cast<SqliteCreateTable::Constraint*>(constraint.data());

    // Name
    if (!constr->name.isNull())
    {
        ui->namedCheckBox->setChecked(true);
        ui->namedLineEdit->setText(constr->name);
    }

    // On conflict
    if (constr->onConflict != SqliteConflictAlgo::null)
    {
        ui->conflictCheckBox->setChecked(true);
        ui->conflictComboBox->setCurrentText(sqliteConflictAlgo(constr->onConflict));
    }

    // Columns
    int idx;
    QCheckBox* check = nullptr;
    QComboBox* combo = nullptr;
    for (SqliteIndexedColumn* idxCol : constr->indexedColumns)
    {
        idx = getColumnIndex(idxCol->name);
        if (idx < 0)
            continue;

        check = dynamic_cast<QCheckBox*>(columnsLayout->itemAtPosition(idx, 0)->widget());
        check->setChecked(true);

        combo = dynamic_cast<QComboBox*>(columnsLayout->itemAtPosition(idx, 1)->widget());
        combo->setCurrentText(idxCol->collate);

        combo = dynamic_cast<QComboBox*>(columnsLayout->itemAtPosition(idx, 2)->widget());
        combo->setCurrentText(sqliteSortOrder(idxCol->sortOrder));
    }
}

void TablePrimaryKeyAndUniquePanel::buildColumns()
{
    totalColumns = 0;
    if (constraint.isNull())
        return;

    SqliteCreateTable* createTable = dynamic_cast<SqliteCreateTable*>(constraint->parentStatement());
    int row = 0;
    for (SqliteCreateTable::Column* column : createTable->columns)
        buildColumn(column, row++);
}
