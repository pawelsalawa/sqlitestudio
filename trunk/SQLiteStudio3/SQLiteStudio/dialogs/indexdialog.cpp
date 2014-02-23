#include "indexdialog.h"
#include "ui_indexdialog.h"
#include "schemaresolver.h"
#include "parser/ast/sqliteindexedcolumn.h"
#include "notifymanager.h"
#include "utils_sql.h"
#include "db/chainexecutor.h"
#include "dbtree/dbtree.h"
#include "ddlpreviewdialog.h"
#include "uiconfig.h"
#include "config.h"
#include "uiutils.h"
#include <QDebug>
#include <QGridLayout>
#include <QSignalMapper>
#include <QScrollBar>
#include <QPushButton>
#include <QMessageBox>

IndexDialog::IndexDialog(Db* db, QWidget *parent) :
    QDialog(parent),
    db(db),
    ui(new Ui::IndexDialog)
{
    init();
}

IndexDialog::IndexDialog(Db* db, const QString& index, QWidget* parent) :
    QDialog(parent),
    db(db),
    index(index),
    ui(new Ui::IndexDialog)
{
    existingIndex = true;
    init();
}

IndexDialog::~IndexDialog()
{
    delete ui;
}

void IndexDialog::changeEvent(QEvent *e)
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

void IndexDialog::init()
{
    ui->setupUi(this);
    if (!db || !db->isOpen())
    {
        qCritical() << "Created IndexDialog for null or closed database.";
        notifyError(tr("Tried to open index dialog for closed or inexisting database."));
        reject();
        return;
    }

    ui->scrollArea->setAutoFillBackground(false);
    ui->scrollArea->viewport()->setAutoFillBackground(false);
    ui->columnsWidget->setAutoFillBackground(false);

    ui->partialIndexEdit->setDb(db);

    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));

    columnStateSignalMapping = new QSignalMapper(this);
    connect(columnStateSignalMapping, SIGNAL(mapped(int)), this, SLOT(updateColumnState(int)));

    columnsLayout = dynamic_cast<QGridLayout*>(ui->columnsWidget->layout());

    SchemaResolver resolver(db);
    ui->tableCombo->addItem(QString::null);
    ui->tableCombo->addItems(resolver.getTables());
    connect(ui->tableCombo, SIGNAL(currentTextChanged(QString)), this, SLOT(updateTable(QString)));
    connect(ui->tableCombo, SIGNAL(currentTextChanged(QString)), this, SLOT(updateValidation()));
    if (existingIndex)
        ui->tableCombo->setEnabled(false);

    if (db->getDialect() == Dialect::Sqlite3)
    {
        connect(ui->partialIndexCheck, SIGNAL(toggled(bool)), this, SLOT(updatePartialConditionState()));
        connect(ui->partialIndexEdit, SIGNAL(errorsChecked(bool)), this, SLOT(updateValidation()));
        connect(ui->partialIndexEdit, SIGNAL(textChanged()), this, SLOT(updateValidation()));
        ui->partialIndexEdit->setVirtualSqlExpression("SELECT %1");
        updatePartialConditionState();
    }
    else
    {
        ui->partialIndexCheck->setVisible(false);
        ui->partialIndexEdit->setVisible(false);
    }

    readCollations();

    ui->ddlEdit->setSqliteVersion(db->getVersion());

    if (index.isNull())
        createIndex = SqliteCreateIndexPtr::create();
    else
        readIndex();

    originalCreateIndex = SqliteCreateIndexPtr::create(*createIndex);

    ui->nameEdit->setText(index);
    setTable(createIndex->table);

    if (!index.isNull())
        applyIndex();

    updateValidation();

    adjustSize();

    ui->nameEdit->setFocus();
}

void IndexDialog::readIndex()
{
    SchemaResolver resolver(db);
    SqliteQueryPtr parsedObject = resolver.getParsedObject(index);
    if (!parsedObject.dynamicCast<SqliteCreateIndex>())
    {
        notifyError(tr("Could not process index %1 correctly. Unable to open an index dialog.").arg(index));
        reject();
        return;
    }

    createIndex = parsedObject.dynamicCast<SqliteCreateIndex>();
}

void IndexDialog::buildColumns()
{
    totalColumns = 0;

    // Clean up
    foreach (QCheckBox* cb, columnCheckBoxes)
        delete cb;

    foreach (QComboBox* cb, sortComboBoxes)
        delete cb;

    foreach (QComboBox* cb, collateComboBoxes)
        delete cb;

    columnCheckBoxes.clear();
    sortComboBoxes.clear();
    collateComboBoxes.clear();

    int row = 0;
    foreach (const QString& column, tableColumns)
        buildColumn(column, row++);
}

void IndexDialog::updateTable(const QString& value)
{
    table = value;

    SchemaResolver resolver(db);
    tableColumns = resolver.getTableColumns(table);

    buildColumns();
}

void IndexDialog::updateValidation()
{
    bool tableOk = ui->tableCombo->currentIndex() > 0;
    bool colSelected = false;

    if (tableOk)
    {
        foreach (QCheckBox* cb, columnCheckBoxes)
        {
            if (cb->isChecked())
            {
                colSelected = true;
                break;
            }
        }
    }

    bool partialConditionOk = (!ui->partialIndexCheck->isChecked() ||
                           (ui->partialIndexEdit->isSyntaxChecked() && !ui->partialIndexEdit->haveErrors()));

    setValidStyle(ui->tableLabel, tableOk);
    setValidStyle(ui->hdrColumnLabel, colSelected);
    foreach (QCheckBox* cb, columnCheckBoxes)
        setValidStyle(cb, colSelected);

    setValidStyle(ui->partialIndexCheck, partialConditionOk);

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(colSelected && partialConditionOk);
}

void IndexDialog::setTable(const QString& value)
{
    ui->tableCombo->setCurrentText(value);
}

void IndexDialog::readCollations()
{
    SchemaResolver resolver(db);
    QStringList collList = resolver.getCollations();

    if (collList.size() > 0)
        collList.prepend("");

    collations.setStringList(collList);
}

void IndexDialog::buildColumn(const QString& name, int row)
{
    int col = 0;

    QCheckBox* check = new QCheckBox(name);
    columnsLayout->addWidget(check, row, col++);
    columnStateSignalMapping->setMapping(check, row);
    connect(check, SIGNAL(toggled(bool)), columnStateSignalMapping, SLOT(map()));
    connect(check, SIGNAL(toggled(bool)), this, SLOT(updateValidation()));
    columnCheckBoxes << check;

    QComboBox* collation = nullptr;
    if (db->getDialect() == Dialect::Sqlite3)
    {
        collation = new QComboBox();
        collation->setMaximumWidth(ui->hdrCollateLabel->width());
        collation->setMinimumWidth(ui->hdrCollateLabel->width() - ui->scrollArea->verticalScrollBar()->width());
        collation->setEditable(true);
        collation->lineEdit()->setPlaceholderText(tr("Collate", "index dialog"));
        collation->setModel(&collations);
        columnsLayout->addWidget(collation, row, col++);
        collateComboBoxes << collation;
    }

    QComboBox* sortOrder = new QComboBox();
    sortOrder->setFixedWidth(ui->hdrSortLabel->width());
    sortOrder->setToolTip(tr("Sort order", "table constraints"));
    columnsLayout->addWidget(sortOrder, row, col++);
    sortComboBoxes << sortOrder;

    QStringList sortList = {"", sqliteSortOrder(SqliteSortOrder::ASC), sqliteSortOrder(SqliteSortOrder::DESC)};
    sortOrder->addItems(sortList);

    totalColumns++;

    updateColumnState(row);
}

void IndexDialog::updateColumnState(int row)
{
    bool enabled = columnCheckBoxes[row]->isChecked();
    sortComboBoxes[row]->setEnabled(enabled);
    if (db->getDialect() == Dialect::Sqlite3)
        collateComboBoxes[row]->setEnabled(enabled);
}

void IndexDialog::updatePartialConditionState()
{
    ui->partialIndexEdit->setEnabled(ui->partialIndexCheck->isChecked());
    updateValidation();
}

void IndexDialog::updateDdl()
{
    rebuildCreateIndex();
    ui->ddlEdit->setPlainText(createIndex->detokenize());
}

void IndexDialog::tabChanged(int tab)
{
    if (tab == 1)
        updateDdl();
}

void IndexDialog::applyColumnValues()
{
    Dialect dialect = db->getDialect();
    int row;
    foreach (SqliteIndexedColumn* idxCol, createIndex->indexedColumns)
    {
        row = indexOf(tableColumns, idxCol->name, Qt::CaseInsensitive);
        if (row == -1)
        {
            qCritical() << "Cannot find column from index in the table columns! Indexed column:" << idxCol->name
                        << ", table columns:" << tableColumns << ", index name:" << index << ", table name:" << table;
            continue;
        }

        columnCheckBoxes[row]->setChecked(true);
        updateColumnState(row);
        sortComboBoxes[row]->setCurrentText(sqliteSortOrder(idxCol->sortOrder));
        if (dialect == Dialect::Sqlite3)
            collateComboBoxes[row]->setCurrentText(idxCol->collate);
    }
}

void IndexDialog::applyIndex()
{
    applyColumnValues();

    ui->partialIndexCheck->setChecked(createIndex->where != nullptr);
    if (createIndex->where)
        ui->partialIndexEdit->setPlainText(createIndex->where->detokenize());
}

SqliteIndexedColumn* IndexDialog::addIndexedColumn(const QString& name)
{
    SqliteIndexedColumn* idxCol = new SqliteIndexedColumn();
    idxCol->name = name;
    idxCol->setParent(createIndex.data());
    createIndex->indexedColumns << idxCol;
    return idxCol;
}

void IndexDialog::rebuildCreateIndex()
{
    createIndex = SqliteCreateIndexPtr::create();
    createIndex->index = ui->nameEdit->text();
    if (ui->tableCombo->currentIndex() > -1)
        createIndex->table = ui->tableCombo->currentText();

    createIndex->uniqueKw = ui->uniqueCheck->isChecked();

    SqliteIndexedColumn* idxCol;
    int i = -1;
    foreach (const QString& column, tableColumns)
    {
        i++;

        if (!columnCheckBoxes[i]->isChecked())
            continue;

        idxCol = addIndexedColumn(column);
        if (!collateComboBoxes[i]->currentText().isEmpty())
            idxCol->collate = collateComboBoxes[i]->currentText();

        if (sortComboBoxes[i]->currentIndex() > 0)
            idxCol->sortOrder = sqliteSortOrder(sortComboBoxes[i]->currentText());
    }

    if (ui->partialIndexCheck->isChecked())
    {
        if (createIndex->where)
            delete createIndex->where;

        Parser parser(db->getDialect());
        SqliteExpr* expr = parser.parseExpr(ui->partialIndexEdit->toPlainText());

        if (expr)
        {
            expr->setParent(createIndex.data());
            createIndex->where = expr;
        }
        else
        {
            qCritical() << "Could not parse expression from partial index condition: " << ui->partialIndexEdit->toPlainText()
                        << ", the CREATE INDEX statement will be incomplete.";
        }
    }

    createIndex->rebuildTokens();
}

void IndexDialog::accept()
{
    rebuildCreateIndex();

    Dialect dialect = db->getDialect();

    QStringList sqls;
    if (existingIndex)
        sqls << QString("DROP INDEX %1").arg(wrapObjIfNeeded(originalCreateIndex->index, dialect));

    sqls << createIndex->detokenize();

    if (!CFG_UI.General.DontShowDdlPreview.get())
    {
        DdlPreviewDialog dialog(db->getDialect(), this);
        dialog.setDdl(sqls);
        if (dialog.exec() != QDialog::Accepted)
            return;
    }

    ChainExecutor executor;
    executor.setDb(db);
    executor.setAsync(false);
    executor.setQueries(sqls);
    executor.exec();

    if (executor.getSuccessfulExecution())
    {
        CFG->addDdlHistory(sqls.join(";\n"), db->getName(), db->getPath());

        QDialog::accept();
        DBTREE->refreshSchema(db);
        return;
    }

    QMessageBox::critical(this, tr("Error", "index dialog"), tr("An error occurred while executing SQL statements:\n%1")
                          .arg(executor.getExecutionErrors().join(",\n")), QMessageBox::Ok);
}
