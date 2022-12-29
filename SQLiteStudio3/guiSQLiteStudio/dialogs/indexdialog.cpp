#include "indexdialog.h"
#include "ui_indexdialog.h"
#include "schemaresolver.h"
#include "services/notifymanager.h"
#include "common/utils_sql.h"
#include "db/chainexecutor.h"
#include "dbtree/dbtree.h"
#include "ddlpreviewdialog.h"
#include "uiconfig.h"
#include "services/config.h"
#include "uiutils.h"
#include "db/sqlite3.h"
#include "indexexprcolumndialog.h"
#include "windows/editorwindow.h"
#include "services/codeformatter.h"
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

    bool sysIdx = isSystemIndex(index);
    ui->indexTab->setDisabled(sysIdx);
    ui->ddlTab->setDisabled(sysIdx);
}

IndexDialog::~IndexDialog()
{
    clearColumns();
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
    limitDialogWidth(this);
    if (!db || !db->isOpen())
    {
        qCritical() << "Created IndexDialog for null or closed database.";
        notifyError(tr("Tried to open index dialog for closed or inexisting database."));
        preReject();
        return;
    }

    ui->moveUpButton->setIcon(ICONS.MOVE_UP);
    ui->moveDownButton->setIcon(ICONS.MOVE_DOWN);
    ui->addExprColumnButton->setIcon(ICONS.INDEX_EXPR_ADD);
    ui->editExprColumnButton->setIcon(ICONS.INDEX_EXPR_EDIT);
    ui->delExprColumnButton->setIcon(ICONS.INDEX_EXPR_DEL);
    connect(ui->moveUpButton, SIGNAL(clicked()), this, SLOT(moveColumnUp()));
    connect(ui->moveDownButton, SIGNAL(clicked()), this, SLOT(moveColumnDown()));
    connect(ui->addExprColumnButton, SIGNAL(clicked()), this, SLOT(addExprColumn()));
    connect(ui->editExprColumnButton, SIGNAL(clicked()), this, SLOT(editExprColumn()));
    connect(ui->delExprColumnButton, SIGNAL(clicked()), this, SLOT(delExprColumn()));
    connect(ui->columnsTable, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(editExprColumn(int)));

    ui->columnsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    connect(ui->columnsTable->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), this, SLOT(updateToolBarButtons(QModelIndex)));

    ui->partialIndexEdit->setDb(db);

    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));

    columnStateSignalMapping = new QSignalMapper(this);
    connect(columnStateSignalMapping, SIGNAL(mapped(QString)), this, SLOT(updateColumnState(QString)));

    SchemaResolver resolver(db);
    ui->tableCombo->addItem(QString());
    ui->tableCombo->addItems(resolver.getTables());
    connect(ui->tableCombo, SIGNAL(currentTextChanged(QString)), this, SLOT(updateTable(QString)));
    connect(ui->tableCombo, SIGNAL(currentTextChanged(QString)), this, SLOT(updateValidation()));
    connect(ui->uniqueCheck, SIGNAL(toggled(bool)), this, SLOT(updateValidation()));
    if (existingIndex)
        ui->tableCombo->setEnabled(false);

    connect(ui->partialIndexCheck, SIGNAL(toggled(bool)), this, SLOT(updatePartialConditionState()));
    connect(ui->partialIndexEdit, SIGNAL(errorsChecked(bool)), this, SLOT(updateValidation()));
    connect(ui->partialIndexEdit, SIGNAL(textChanged()), this, SLOT(updateValidation()));
    ui->partialIndexEdit->setVirtualSqlExpression("SELECT %1");
    updatePartialConditionState();
    ui->columnsTable->setColumnHidden(2, false);

    readCollations();

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

    ui->nameEdit->setFocus();
}

void IndexDialog::readIndex()
{
    SchemaResolver resolver(db);
    SqliteQueryPtr parsedObject = resolver.getParsedObject(index, SchemaResolver::INDEX);
    if (!parsedObject.dynamicCast<SqliteCreateIndex>())
    {
        createIndex = SqliteCreateIndexPtr::create();
        notifyError(tr("Could not process index %1 correctly. Unable to open an index dialog.").arg(index));
        preReject();
        return;
    }

    createIndex = parsedObject.dynamicCast<SqliteCreateIndex>();
}

void IndexDialog::buildColumns()
{
    // Clean up
    clearColumns();
    ui->columnsTable->setRowCount(0);

    int row = 0;
    for (const QString& column : tableColumns)
        buildColumn(column, row++);

    updateToolBarButtons();
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
    bool hasExprColumn = false;

    if (tableOk)
    {
        for (Column* col : columns.values())
        {
            if (col->getCheck()->isChecked())
            {
                colSelected = true;
                hasExprColumn |= col->isExpr();
            }
        }
    }

    bool partialConditionOk = (!ui->partialIndexCheck->isChecked() ||
                           (ui->partialIndexEdit->isSyntaxChecked() && !ui->partialIndexEdit->haveErrors()));

    bool uniqueAndExprOk = !(ui->uniqueCheck->isChecked() && hasExprColumn);

    setValidState(ui->uniqueCheck, uniqueAndExprOk, tr("Unique index cannot have indexed expressions. Either remove expressions from list below, or uncheck this option."));
    setValidState(ui->tableCombo, tableOk, tr("Pick the table for the index."));
    setValidState(ui->columnsTable, colSelected, tr("Select at least one column."));
    setValidState(ui->partialIndexCheck, partialConditionOk, tr("Enter a valid condition."));

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(colSelected && partialConditionOk && uniqueAndExprOk);
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
    Column* column = new Column(name, ui->columnsTable);
    buildColumn(column, row);
}

IndexDialog::Column* IndexDialog::buildColumn(SqliteOrderBy* orderBy, int row)
{
    Column* column = orderBy->isSimpleColumn() ?
                new Column(orderBy->getColumnName(), ui->columnsTable) :
                new Column(dynamic_cast<SqliteExpr*>(orderBy->expr->clone()), ui->columnsTable);

    buildColumn(column, row);
    return column;
}

IndexDialog::Column* IndexDialog::buildColumn(SqliteExpr* expr, int row)
{
    Column* column = new Column(expr, ui->columnsTable);
    buildColumn(column, row);
    return column;
}

void IndexDialog::buildColumn(Column* column, int row)
{
    totalColumns++;
    ui->columnsTable->setRowCount(totalColumns);

    QString key = column->getKey();
    columns[key] = column;
    columnsByRow << column;

    column->setCheckParent(new QWidget());
    QHBoxLayout* layout = new QHBoxLayout();
    QMargins margins = layout->contentsMargins();
    margins.setTop(0);
    margins.setBottom(0);
    margins.setLeft(4);
    margins.setRight(4);
    layout->setContentsMargins(margins);
    column->getCheckParent()->setLayout(layout);

    column->setCheck(new QCheckBox(column->getKey()));
    column->getCheckParent()->layout()->addWidget(column->getCheck());

    columnStateSignalMapping->setMapping(column->getCheck(), key);
    connect(column->getCheck(), SIGNAL(toggled(bool)), columnStateSignalMapping, SLOT(map()));
    connect(column->getCheck(), SIGNAL(toggled(bool)), this, SLOT(updateValidation()));

    column->setCollation(new QComboBox());
    column->getCollation()->setEditable(true);
    column->getCollation()->lineEdit()->setPlaceholderText(tr("default", "index dialog"));
    column->getCollation()->setModel(&collations);

    column->setSort(new QComboBox());
    column->getSort()->setToolTip(tr("Sort order", "table constraints"));

    QStringList sortList = {"", sqliteSortOrder(SqliteSortOrder::ASC), sqliteSortOrder(SqliteSortOrder::DESC)};
    column->getSort()->addItems(sortList);

    column->prepareForNewRow();
    column->assignToNewRow(row);

    updateColumnState(key);
}

void IndexDialog::updateColumnState(const QString& columnKey)
{
    Column* col = columns[columnKey];

    bool enabled = col->getCheck()->isChecked();
    col->getSort()->setEnabled(enabled);
    col->getCollation()->setEnabled(enabled);
}

void IndexDialog::updatePartialConditionState()
{
    ui->partialIndexEdit->setEnabled(ui->partialIndexCheck->isChecked());
    updateValidation();
}

void IndexDialog::updateDdl()
{
    rebuildCreateIndex();
    QString formatted = FORMATTER->format("sql", createIndex->detokenize(), db);
    ui->ddlEdit->setPlainText(formatted);
}

void IndexDialog::tabChanged(int tab)
{
    if (tab == 1)
        updateDdl();
}

void IndexDialog::moveColumnUp()
{
    QModelIndex idx = ui->columnsTable->selectionModel()->currentIndex();
    if (!idx.isValid())
        return;

    int row = idx.row();
    if (row <= 0)
        return;

    columnsByRow.move(row, row - 1);
    rebuildColumnsByNewOrder();

    idx = ui->columnsTable->model()->index(row - 1, 0);
    ui->columnsTable->selectionModel()->setCurrentIndex(idx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
}

void IndexDialog::moveColumnDown()
{
    QModelIndex idx = ui->columnsTable->selectionModel()->currentIndex();
    if (!idx.isValid())
        return;

    int row = idx.row();
    int cols = tableColumns.size();

    if ((row + 1) >= cols)
        return;

    columnsByRow.move(row, row + 1);
    rebuildColumnsByNewOrder();

    idx = ui->columnsTable->model()->index(row + 1, 0);
    ui->columnsTable->selectionModel()->setCurrentIndex(idx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
}

void IndexDialog::updateToolBarButtons(const QModelIndex& idx)
{
    if (!idx.isValid())
    {
        ui->editExprColumnButton->setEnabled(false);
        ui->delExprColumnButton->setEnabled(false);
        ui->moveUpButton->setEnabled(false);
        ui->moveDownButton->setEnabled(false);
        return;
    }

    int row = idx.row();
    int cols = tableColumns.size();
    ui->moveUpButton->setEnabled(row > 0);
    ui->moveDownButton->setEnabled((row + 1) < cols);

    bool isExpr = columnsByRow[row]->isExpr();
    ui->editExprColumnButton->setEnabled(isExpr);
    ui->delExprColumnButton->setEnabled(isExpr);
}

void IndexDialog::addExprColumn()
{
    IndexExprColumnDialog dialog(db, this);
    dialog.setExistingExprColumnKeys(getExistingColumnExprs());
    dialog.setTableColumns(getTableColumns());
    if (!dialog.exec())
        return;

    SqliteExpr* expr = dialog.getColumn();
    if (!expr)
    {
        qCritical() << "Null expr in IndexDialog::addExprColumn(). Aborting.";
        return;
    }

    int row = columnsByRow.size();
    ui->columnsTable->insertRow(row);

    Column* col = buildColumn(expr, row);
    col->getCheck()->setChecked(true);
    rebuildColumnsByNewOrder();

    ui->columnsTable->scrollToBottom();
    updateValidation();
}

void IndexDialog::editExprColumn(int row)
{
    if (row < 0)
        row = ui->columnsTable->currentRow();

    if (row < 0 || row >= columnsByRow.size())
    {
        qWarning() << "IndexDialog::editExprColumn() called for row out of bounds:" << row << "while there are" << columnsByRow.size() << "rows.";
        return;
    }

    Column* col = columnsByRow[row];
    if (!col->isExpr())
    {
        qWarning() << "IndexDialog::editExprColumn() called for non-expr index column.";
        return;
    }

    IndexExprColumnDialog dialog(db, col->getExpr(), this);
    dialog.setExistingExprColumnKeys(getExistingColumnExprs(col->getKey()));
    dialog.setTableColumns(getTableColumns());
    if (!dialog.exec())
        return;

    SqliteExpr* expr = dialog.getColumn();
    if (!expr)
    {
        qCritical() << "Null expr in IndexDialog::editExprColumn(). Aborting.";
        return;
    }

    QString oldKey = col->getKey();
    col->setExpr(expr);
    QString newKey = col->getKey();

    columns.remove(oldKey);
    columns[newKey] = col;

    col->getCheck()->setText(newKey);
    col->getCheck()->setChecked(true);
    rebuildColumnsByNewOrder();
    updateValidation();
}

void IndexDialog::delExprColumn()
{
    int row = ui->columnsTable->currentRow();

    if (row < 0 || row >= columnsByRow.size())
    {
        qWarning() << "IndexDialog::delExprColumn() called for row out of bounds:" << row << "while there are" << columnsByRow.size() << "rows.";
        return;
    }

    Column* col = columnsByRow[row];
    if (!col->isExpr())
    {
        qWarning() << "IndexDialog::delExprColumn() called for non-expr index column.";
        return;
    }

    // Removes all widgets in the row
    ui->columnsTable->removeRow(row);

    columnsByRow.removeOne(col);
    columns.remove(col->getKey());
    delete col;

    rebuildColumnsByNewOrder();
    updateValidation();
}

void IndexDialog::applyColumnValues()
{
    Column* column = nullptr;
    QString key;
    int row = 0;
    int totalRows = tableColumns.size();
    bool orderChanged = false;
    QStringList orderedIndexKeys;
    for (SqliteOrderBy* idxCol : createIndex->indexedColumns)
    {
        key = getKey(idxCol);
        orderedIndexKeys << key;

        if (idxCol->isSimpleColumn())
        {
            column = columns[key];
            if (!column)
            {
                qCritical() << "Cannot find column by name or expression! Column name/expression:" << key
                            << ", available columns:" << columns.keys() << ", index name:" << index;
                continue;
            }
        }
        else
            column = buildColumn(idxCol, totalRows++);

        column->getCheck()->setChecked(true);
        updateColumnState(key);
        column->getSort()->setCurrentText(sqliteSortOrder(idxCol->order));
        column->getCollation()->setCurrentText(idxCol->getCollation());

        // Setting proper order
        int intendedRow = columnsByRow.indexOf(column);
        if (intendedRow != row)
        {
            columnsByRow.move(intendedRow, row);
            orderChanged = true;
        }

        row++;
    }

    if (orderChanged)
        rebuildColumnsByNewOrder();
}

void IndexDialog::applyIndex()
{
    applyColumnValues();

    ui->uniqueCheck->setChecked(createIndex->uniqueKw);
    ui->partialIndexCheck->setChecked(createIndex->where != nullptr);
    if (createIndex->where)
        ui->partialIndexEdit->setPlainText(createIndex->where->detokenize());
}

SqliteOrderBy* IndexDialog::addIndexedColumn(const QString& name)
{
    SqliteOrderBy* idxCol = new SqliteOrderBy();
    idxCol->setParent(createIndex.data());

    SqliteExpr* expr = new SqliteExpr();
    expr->initId(name);
    idxCol->expr = expr;
    expr->setParent(idxCol);

    createIndex->indexedColumns << idxCol;
    return idxCol;
}

SqliteOrderBy* IndexDialog::addIndexedColumn(SqliteExpr* expr)
{
    SqliteOrderBy* idxCol = new SqliteOrderBy();
    idxCol->setParent(createIndex.data());

    SqliteExpr* clonedExpr = dynamic_cast<SqliteExpr*>(expr->clone());
    idxCol->expr = clonedExpr;
    clonedExpr->setParent(idxCol);

    createIndex->indexedColumns << idxCol;
    return idxCol;
}

void IndexDialog::addCollation(SqliteOrderBy* col, const QString& name)
{
    SqliteExpr* expr = new SqliteExpr();
    col->expr->setParent(expr);
    expr->initCollate(col->expr, name);
    expr->setParent(col);

    col->expr = expr;
}

void IndexDialog::rebuildCreateIndex()
{
    createIndex = SqliteCreateIndexPtr::create();
    createIndex->index = ui->nameEdit->text();
    if (ui->tableCombo->currentIndex() > -1)
        createIndex->table = ui->tableCombo->currentText();

    createIndex->uniqueKw = ui->uniqueCheck->isChecked();

    SqliteOrderBy* idxCol = nullptr;
    for (Column* column : columnsByRow)
    {
        if (!column->getCheck()->isChecked())
            continue;

        if (column->isExpr())
            idxCol = addIndexedColumn(column->getExpr());
        else
            idxCol = addIndexedColumn(column->getName());

        if (!column->getCollation()->currentText().isEmpty())
            addCollation(idxCol, column->getCollation()->currentText());

        if (column->getSort()->currentIndex() > 0)
            idxCol->order = sqliteSortOrder(column->getSort()->currentText());
    }

    if (ui->partialIndexCheck->isChecked())
    {
        if (createIndex->where)
            delete createIndex->where;

        Parser parser;
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

void IndexDialog::queryDuplicates()
{
    static QString queryTpl = QStringLiteral("SELECT %1 FROM %2 GROUP BY %3 HAVING %4;\n");
    static QString countTpl = QStringLiteral("count(%1) AS %2");
    static QString countColNameTpl = QStringLiteral("count(%1)");
    static QString countConditionTpl = QStringLiteral("count(%1) > 1");

    QStringList cols;
    QStringList grpCols;
    QStringList countCols;
    QString wrappedCol;
    QString countColName;
    for (const QString& column : tableColumns)
    {
        if (!columns[column]->getCheck()->isChecked())
            continue;

        wrappedCol = wrapObjIfNeeded(column);
        cols << wrappedCol;
        grpCols << wrappedCol;
        countColName = wrapObjIfNeeded(countColNameTpl.arg(column));
        cols << countTpl.arg(wrappedCol, countColName);
        countCols << countConditionTpl.arg(wrappedCol);
    }

    EditorWindow* editor = MAINWINDOW->openSqlEditor();
    editor->setCurrentDb(db);

    QString sqlCols = cols.join(", ");
    QString sqlGrpCols = grpCols.join(", ");
    QString sqlCntCols = countCols.join(" AND ");
    QString sqlTable = wrapObjIfNeeded(ui->tableCombo->currentText());
    editor->setContents(queryTpl.arg(sqlCols, sqlTable, sqlGrpCols, sqlCntCols));
    editor->execute();
}

void IndexDialog::clearColumns()
{
    for (Column* c : columns.values())
        delete c;

    columns.clear();
    columnsByRow.clear();
}

void IndexDialog::rebuildColumnsByNewOrder()
{
    int row = 0;
    for (Column* column : columnsByRow)
    {
        column->prepareForNewRow();
        column->assignToNewRow(row++);
    }
}

QString IndexDialog::getKey(SqliteOrderBy* col) const
{
    if (col->isSimpleColumn())
        return col->getColumnName();

    return buildKey(col->expr);
}

QString IndexDialog::buildKey(SqliteExpr* expr)
{
    if (expr->mode == SqliteExpr::Mode::COLLATE && expr->expr1)
        return expr->expr1->tokens.filterWhiteSpaces(false).detokenize().trimmed();

    return expr->tokens.filterWhiteSpaces(false).detokenize().trimmed();
}

QStringList IndexDialog::getExistingColumnExprs(const QString& exceptThis) const
{
    QString key;
    QStringList exprs;
    for (Column* col : columnsByRow)
    {
        if (col->isExpr())
        {
            key = col->getKey();
            if (!exceptThis.isNull() && key == exceptThis)
                continue;

            exprs << key;
        }
    }
    return exprs;
}

QStringList IndexDialog::getTableColumns() const
{
    QStringList cols;
    for (Column* col : columnsByRow)
    {
        if (!col->isExpr())
            cols << col->getKey();
    }
    return cols;
}

void IndexDialog::preReject()
{
    preRejected = true;
}

QString IndexDialog::getOriginalDdl() const
{
    SqliteCreateIndex* initialCreateIndex = originalCreateIndex->typeClone<SqliteCreateIndex>();
    initialCreateIndex->rebuildTokens();
    QString initialDdl = initialCreateIndex->detokenize();
    delete initialCreateIndex;
    return initialDdl;
}

void IndexDialog::accept()
{
    QString initialDdl = getOriginalDdl();
    rebuildCreateIndex();
    QString ddl = createIndex->detokenize();
    if (initialDdl == ddl)
    {
        // Nothing changed. Just close.
        QDialog::accept();
        return;
    }

    QStringList sqls;
    if (existingIndex)
        sqls << QString("DROP INDEX %1").arg(wrapObjIfNeeded(originalCreateIndex->index));

    sqls << ddl;

    if (!CFG_UI.General.DontShowDdlPreview.get())
    {
        DdlPreviewDialog dialog(db, this);
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
        CFG->addDdlHistory(sqls.join("\n"), db->getName(), db->getPath());

        QDialog::accept();
        DBTREE->refreshSchema(db);
        return;
    }

    if (executor.getErrors().size() == 1 && executor.getErrors().first().first == SQLITE_CONSTRAINT)
    {
        int res = QMessageBox::critical(this,
                                        tr("Error", "index dialog"),
                                        tr("Cannot create unique index, because values in selected columns are not unique. "
                                           "Would you like to execute SELECT query to see problematic values?"),
                                        QMessageBox::Yes,
                                        QMessageBox::No);
        if (res == QMessageBox::Yes)
        {
            QDialog::reject();
            queryDuplicates();
        }
    }
    else
    {
        QMessageBox::critical(this, tr("Error", "index dialog"), tr("An error occurred while executing SQL statements:\n%1")
                              .arg(executor.getErrorsMessages().join(",\n")), QMessageBox::Ok);
    }
}

int IndexDialog::exec()
{
    if (preRejected)
        return Rejected;

    return QDialog::exec();
}

IndexDialog::Column::Column(const QString& name, QTableWidget* table)
{
    this->name = name;
    this->table = table;
}

IndexDialog::Column::Column(SqliteExpr* expr, QTableWidget* table)
{
    this->expr = expr;
    this->table = table;
}

IndexDialog::Column::~Column()
{
    safe_delete(expr);
}

void IndexDialog::Column::assignToNewRow(int row)
{
    table->setCellWidget(row, 0, column1Contrainer);
    table->setCellWidget(row, 1, column2Contrainer);
    table->setCellWidget(row, 2, column3Contrainer);
}

void IndexDialog::Column::prepareForNewRow()
{
    column1Contrainer = defineContainer(checkParent);
    column2Contrainer = defineContainer(sort);
    column3Contrainer = defineContainer(collation);
}

QCheckBox* IndexDialog::Column::getCheck() const
{
    return check;
}

void IndexDialog::Column::setCheck(QCheckBox* cb)
{
    check = cb;
}

QWidget* IndexDialog::Column::getCheckParent() const
{
    return checkParent;
}

void IndexDialog::Column::setCheckParent(QWidget* w)
{
    checkParent = w;
}

QComboBox* IndexDialog::Column::getSort() const
{
    return sort;
}

void IndexDialog::Column::setSort(QComboBox* cb)
{
    sort = cb;
}

QComboBox* IndexDialog::Column::getCollation() const
{
    return collation;
}

void IndexDialog::Column::setCollation(QComboBox* cb)
{
    collation = cb;
}

QString IndexDialog::Column::getName() const
{
    return name;
}

SqliteExpr* IndexDialog::Column::getExpr() const
{
    // If column's expression contains collation at top level,
    // the EXPR for processing is inner expr of collation.
    if (expr->mode == SqliteExpr::Mode::COLLATE)
        return expr->expr1;

    return expr;
}

void IndexDialog::Column::setExpr(SqliteExpr* expr)
{
    safe_delete(this->expr);
    this->expr = expr;
}

bool IndexDialog::Column::isExpr() const
{
    return expr != nullptr;
}

QString IndexDialog::Column::getKey() const
{
    if (expr)
        return IndexDialog::buildKey(expr);
    else
        return name;
}

QWidget* IndexDialog::Column::defineContainer(QWidget* w)
{
    QHBoxLayout* layout = new QHBoxLayout();
    QMargins margins = layout->contentsMargins();
    margins.setTop(0);
    margins.setBottom(0);
    margins.setLeft(0);
    margins.setRight(0);
    layout->setContentsMargins(margins);

    QWidget* container = new QWidget();
    container->setLayout(layout);
    container->layout()->addWidget(w);
    return container;
}
