#include "tablewindow.h"
#include "ui_tablewindow.h"
#include "services/dbmanager.h"
#include "services/notifymanager.h"
#include "sqlitestudio.h"
#include "schemaresolver.h"
#include "iconmanager.h"
#include "common/intvalidator.h"
#include "common/extlineedit.h"
#include "datagrid/sqltablemodel.h"
#include "common/extaction.h"
#include "mainwindow.h"
#include "tablestructuremodel.h"
#include "tableconstraintsmodel.h"
#include "dialogs/columndialog.h"
#include "dialogs/constraintdialog.h"
#include "mdiarea.h"
#include "sqlitesyntaxhighlighter.h"
#include "dialogs/newconstraintdialog.h"
#include "db/chainexecutor.h"
#include "common/widgetcover.h"
#include "mdiwindow.h"
#include "dbtree/dbtree.h"
#include "constrainttabmodel.h"
#include "parser/ast/sqlitecreateindex.h"
#include "parser/ast/sqlitecreatetrigger.h"
#include "dialogs/messagelistdialog.h"
#include "sqlformatter.h"
#include "uiconfig.h"
#include "dialogs/ddlpreviewdialog.h"
#include "services/config.h"
#include "dbobjectdialogs.h"
#include "dialogs/exportdialog.h"
#include <QMenu>
#include <QToolButton>
#include <QLabel>
#include <QDebug>
#include <QMessageBox>
#include <tablemodifier.h>
#include <QProgressBar>
#include <QPushButton>

// TODO extend QTableView for columns and constraints, so they show full-row-width drop indicator,
// instead of single column drop indicator.

TableWindow::TableWindow(QWidget* parent) :
    MdiChild(parent),
    ui(new Ui::TableWindow)
{
    init();
    applyInitialTab();
}

TableWindow::TableWindow(Db* db, QWidget* parent) :
    MdiChild(parent),
    db(db),
    ui(new Ui::TableWindow)
{
    newTable();
    init();
    initDbAndTable();
    applyInitialTab();
}

TableWindow::TableWindow(const TableWindow& win) :
    MdiChild(win.parentWidget()),
    db(win.db),
    database(win.database),
    table(win.table),
    ui(new Ui::TableWindow)
{
    init();
    initDbAndTable();
    applyInitialTab();
}

TableWindow::TableWindow(QWidget *parent, Db* db, const QString& database, const QString& table) :
    MdiChild(parent),
    db(db),
    database(database),
    table(table),
    ui(new Ui::TableWindow)
{
    init();
    initDbAndTable();
    applyInitialTab();
}

TableWindow::~TableWindow()
{
    delete ui;

    if (tableModifier)
    {
        delete tableModifier;
        tableModifier = nullptr;
    }
}

void TableWindow::staticInit()
{
    qRegisterMetaType<TableWindow>("TableWindow");
}

void TableWindow::newTable()
{
    existingTable = false;
    table = "";
}

void TableWindow::init()
{
    ui->setupUi(this);
    ui->structureSplitter->setStretchFactor(0, 2);

    dataModel = new SqlTableModel(this);
    ui->dataView->init(dataModel);

    initActions();

    connect(dataModel, &SqlQueryModel::executionSuccessful, this, &TableWindow::executionSuccessful);
    connect(dataModel, &SqlQueryModel::executionFailed, this, &TableWindow::executionFailed);
    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));
    connect(this, SIGNAL(modifyStatusChanged()), this, SLOT(updateStructureCommitState()));
    connect(this, SIGNAL(modifyStatusChanged()), this, SLOT(updateBlankNameWarningState()));
    connect(ui->tableNameEdit, SIGNAL(textChanged(QString)), this, SIGNAL(modifyStatusChanged()));
    connect(ui->tableNameEdit, SIGNAL(textChanged(QString)), this, SLOT(nameChanged()));
    connect(ui->indexList, SIGNAL(itemSelectionChanged()), this, SLOT(updateIndexesState()));
    connect(ui->triggerList, SIGNAL(itemSelectionChanged()), this, SLOT(updateTriggersState()));

    structureExecutor = new ChainExecutor(this);
    connect(structureExecutor, SIGNAL(success()), this, SLOT(changesSuccessfullyCommited()));
    connect(structureExecutor, SIGNAL(failure(int,QString)), this, SLOT(changesFailedToCommit(int,QString)));

    setupCoverWidget();

    updateStructureCommitState();
    updateStructureToolbarState();
    updateTableConstraintsToolbarState();
    updateNewTableState();
    updateIndexesState();
    updateTriggersState();
}

void TableWindow::createActions()
{
    createAction(EXPORT, ICONS.TABLE_EXPORT, tr("Export table", "table window"), this, SLOT(exportTable()), this);
    createAction(IMPORT, ICONS.TABLE_IMPORT, tr("Import data to table", "table window"), this, SLOT(importTable()), this);
    createAction(POPULATE, ICONS.TABLE_POPULATE, tr("Populate table", "table window"), this, SLOT(populateTable()), this);

    createStructureActions();
    createDataGridActions();
    createDataFormActions();
    createIndexActions();
    createTriggerActions();

    createAction(NEXT_TAB, "next tab", this, SLOT(nextTab()), this);
    createAction(PREV_TAB, "prev tab", this, SLOT(prevTab()), this);
}

void TableWindow::createStructureActions()
{
    createAction(REFRESH_STRUCTURE, ICONS.RELOAD, tr("Refresh structure", "table window"), this, SLOT(refreshStructure()), ui->structureToolBar);
    ui->structureToolBar->addSeparator();
    createAction(COMMIT_STRUCTURE, ICONS.COMMIT, tr("Commit structure changes", "table window"), this, SLOT(commitStructure()), ui->structureToolBar);
    createAction(ROLLBACK_STRUCTURE, ICONS.ROLLBACK, tr("Rollback structure changes", "table window"), this, SLOT(rollbackStructure()), ui->structureToolBar);
    createAction(ADD_COLUMN, ICONS.TABLE_COLUMN_ADD, tr("Add column", "table window"), this, SLOT(addColumn()), ui->structureToolBar, ui->structureView);
    createAction(EDIT_COLUMN, ICONS.TABLE_COLUMN_EDIT, tr("Edit column", "table window"), this, SLOT(editColumn()), ui->structureToolBar, ui->structureView);
    createAction(DEL_COLUMN, ICONS.TABLE_COLUMN_DELETE, tr("Delete column", "table window"), this, SLOT(delColumn()), ui->structureToolBar, ui->structureView);
    createAction(MOVE_COLUMN_UP, ICONS.MOVE_UP, tr("Move column up", "table window"), this, SLOT(moveColumnUp()), ui->structureToolBar, ui->structureView);
    createAction(MOVE_COLUMN_DOWN, ICONS.MOVE_DOWN, tr("Move column down", "table window"), this, SLOT(moveColumnDown()), ui->structureToolBar, ui->structureView);
    ui->structureToolBar->addSeparator();
    ui->structureToolBar->addAction(actionMap[IMPORT]);
    ui->structureToolBar->addAction(actionMap[EXPORT]);
    ui->structureToolBar->addAction(actionMap[POPULATE]);
    ui->structureToolBar->addSeparator();
    createAction(CREATE_SIMILAR, ICONS.TABLE_CREATE_SIMILAR, tr("Create similar table", "table window"), this, SLOT(createSimilarTable()), ui->structureToolBar);

    // Table constraints
    createAction(ADD_TABLE_CONSTRAINT, ICONS.TABLE_CONSTRAINT_ADD, tr("Add table constraint", "table window"), this, SLOT(addConstraint()), ui->tableConstraintsToolbar, ui->tableConstraintsView);
    createAction(EDIT_TABLE_CONSTRAINT, ICONS.TABLE_CONSTRAINT_EDIT, tr("Edit table constraint", "table window"), this, SLOT(editConstraint()), ui->tableConstraintsToolbar, ui->tableConstraintsView);
    createAction(DEL_TABLE_CONSTRAINT, ICONS.TABLE_COLUMN_DELETE, tr("Delete table constraint", "table window"), this, SLOT(delConstraint()), ui->tableConstraintsToolbar, ui->tableConstraintsView);
    createAction(MOVE_CONSTRAINT_UP, ICONS.MOVE_UP, tr("Move table constraint up", "table window"), this, SLOT(moveConstraintUp()), ui->tableConstraintsToolbar, ui->tableConstraintsView);
    createAction(MOVE_CONSTRAINT_DOWN, ICONS.MOVE_DOWN, tr("Move table constraint down", "table window"), this, SLOT(moveConstraintDown()), ui->tableConstraintsToolbar, ui->tableConstraintsView);
    ui->tableConstraintsToolbar->addSeparator();
    createAction(ADD_TABLE_PK, ICONS.CONSTRAINT_PRIMARY_KEY_ADD, tr("Add table primary key", "table window"), this, SLOT(addPk()), ui->tableConstraintsToolbar, ui->tableConstraintsView);
    createAction(ADD_TABLE_FK, ICONS.CONSTRAINT_FOREIGN_KEY_ADD, tr("Add table foreign key", "table window"), this, SLOT(addFk()), ui->tableConstraintsToolbar, ui->tableConstraintsView);
    createAction(ADD_TABLE_UNIQUE, ICONS.CONSTRAINT_UNIQUE_ADD, tr("Add table unique constraint", "table window"), this, SLOT(addUnique()), ui->tableConstraintsToolbar, ui->tableConstraintsView);
    createAction(ADD_TABLE_CHECK, ICONS.CONSTRAINT_CHECK_ADD, tr("Add table check constraint", "table window"), this, SLOT(addCheck()), ui->tableConstraintsToolbar, ui->tableConstraintsView);
}

void TableWindow::createDataGridActions()
{
    QAction* before = ui->dataView->getAction(DataView::FILTER_VALUE);
    ui->dataView->getGridToolBar()->insertAction(before, actionMap[IMPORT]);
    ui->dataView->getGridToolBar()->insertAction(before, actionMap[EXPORT]);
    ui->dataView->getGridToolBar()->insertAction(before, actionMap[POPULATE]);
    ui->dataView->getGridToolBar()->insertSeparator(before);
}

void TableWindow::createDataFormActions()
{
}

void TableWindow::createIndexActions()
{
    createAction(REFRESH_INDEXES, ICONS.RELOAD, tr("Refresh index list", "table window"), this, SLOT(updateIndexes()), ui->indexToolBar, ui->indexList);
    ui->indexToolBar->addSeparator();
    createAction(ADD_INDEX, ICONS.INDEX_ADD, tr("Create index", "table window"), this, SLOT(addIndex()), ui->indexToolBar, ui->indexList);
    createAction(EDIT_INDEX, ICONS.INDEX_EDIT, tr("Edit index", "table window"), this, SLOT(editIndex()), ui->indexToolBar, ui->indexList);
    createAction(DEL_INDEX, ICONS.INDEX_DEL, tr("Delete index", "table window"), this, SLOT(delIndex()), ui->indexToolBar, ui->indexList);
    connect(ui->indexList, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(editIndex()));
}

void TableWindow::createTriggerActions()
{
    createAction(REFRESH_TRIGGERS, ICONS.RELOAD, tr("Refresh trigger list", "table window"), this, SLOT(updateTriggers()), ui->triggerToolBar, ui->triggerList);
    ui->triggerToolBar->addSeparator();
    createAction(ADD_TRIGGER, ICONS.TRIGGER_ADD, tr("Create trigger", "table window"), this, SLOT(addTrigger()), ui->triggerToolBar, ui->triggerList);
    createAction(EDIT_TRIGGER, ICONS.TRIGGER_EDIT, tr("Edit trigger", "table window"), this, SLOT(editTrigger()), ui->triggerToolBar, ui->triggerList);
    createAction(DEL_TRIGGER, ICONS.TRIGGER_DEL, tr("Delete trigger", "table window"), this, SLOT(delTrigger()), ui->triggerToolBar, ui->triggerList);
    connect(ui->triggerList, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(editTrigger()));
}

void TableWindow::editColumn(const QModelIndex& idx)
{
    if (!idx.isValid())
    {
        qWarning() << "Called TableWindow::editColumn() with invalid index.";
        return;
    }

    SqliteCreateTable::Column* column = structureModel->getColumn(idx.row());
    ColumnDialog columnDialog(db, this);
    columnDialog.setColumn(column);
    if (columnDialog.exec() != QDialog::Accepted)
        return;

    SqliteCreateTable::Column* modifiedColumn = columnDialog.getModifiedColumn();
    structureModel->replaceColumn(idx.row(), modifiedColumn);
    ui->structureView->resizeColumnToContents(0);
}

void TableWindow::delColumn(const QModelIndex& idx)
{
    if (!idx.isValid())
    {
        qWarning() << "Called TableWindow::delColumn() with invalid index.";
        return;
    }

    SqliteCreateTable::Column* column = structureModel->getColumn(idx.row());

    QString msg = tr("Are you sure you want to delete column '%1'?", "table window").arg(column->name);
    int btn = QMessageBox::question(this, tr("Delete column", "table window"), msg);
    if (btn != QMessageBox::Yes)
        return;

    structureModel->delColumn(idx.row());
    ui->structureView->resizeColumnToContents(0);
}

void TableWindow::executeStructureChanges()
{
    QStringList sqls;

    createTable->rebuildTokens();
    if (!existingTable)
    {
        sqls << createTable->detokenize();
    }
    else
    {
        if (tableModifier)
            delete tableModifier;

        tableModifier = new TableModifier(db, database, table);
        tableModifier->alterTable(createTable);

        if (tableModifier->hasMessages())
        {
            MessageListDialog dialog(tr("Following problems will take place while modifying the table.\n"
                                        "Would you like to proceed?", "table window"));
            dialog.setWindowTitle(tr("Table modification", "table window"));
            foreach (const QString& error, tableModifier->getErrors())
                dialog.addError(error);

            foreach (const QString& warn, tableModifier->getWarnings())
                dialog.addWarning(warn);

            if (dialog.exec() != QDialog::Accepted)
                return;
        }

        sqls = tableModifier->generateSqls();
    }

    if (!CFG_UI.General.DontShowDdlPreview.get())
    {
        DdlPreviewDialog dialog(db->getDialect(), this);
        dialog.setDdl(sqls);
        if (dialog.exec() != QDialog::Accepted)
            return;
    }

    structureExecutor->setDb(db);
    structureExecutor->setQueries(sqls);
    structureExecutor->exec();
    widgetCover->show();
    coverCancelButton->setEnabled(true);
}

QModelIndex TableWindow::structureCurrentIndex() const
{
    return ui->structureView->selectionModel()->currentIndex();
}

void TableWindow::updateStructureToolbarState()
{
    QItemSelectionModel *selModel = ui->structureView->selectionModel();
    bool validIdx = false;
    bool isFirst = false;
    bool isLast = false;
    if (selModel)
    {
        QModelIndex currIdx = selModel->currentIndex();
        if (currIdx.isValid())
        {
            validIdx = true;
            if (currIdx.row() == 0)
                isFirst = true;

            if (currIdx.row() == (structureModel->rowCount() - 1))
                isLast = true;
        }
    }

    actionMap[EDIT_COLUMN]->setEnabled(validIdx);
    actionMap[DEL_COLUMN]->setEnabled(validIdx);
    actionMap[MOVE_COLUMN_UP]->setEnabled(validIdx && !isFirst);
    actionMap[MOVE_COLUMN_DOWN]->setEnabled(validIdx && !isLast);
}

void TableWindow::updateStructureCommitState()
{
    bool modified = isModified();
    actionMap[COMMIT_STRUCTURE]->setEnabled(modified);
    actionMap[ROLLBACK_STRUCTURE]->setEnabled(modified);
}

void TableWindow::updateTableConstraintsToolbarState()
{
    QItemSelectionModel *selModel = ui->tableConstraintsView->selectionModel();
    bool anyColumn = structureModel && structureModel->rowCount() > 0;
    bool validIdx = false;
    bool isFirst = false;
    bool isLast = false;
    if (selModel)
    {
        QModelIndex currIdx = selModel->currentIndex();
        if (currIdx.isValid())
        {
            validIdx = true;
            if (currIdx.row() == 0)
                isFirst = true;

            if (currIdx.row() == (structureConstraintsModel->rowCount() - 1))
                isLast = true;
        }
    }

    actionMap[EDIT_TABLE_CONSTRAINT]->setEnabled(anyColumn && validIdx);
    actionMap[DEL_TABLE_CONSTRAINT]->setEnabled(anyColumn && validIdx);
    actionMap[MOVE_CONSTRAINT_UP]->setEnabled(anyColumn && validIdx && !isFirst);
    actionMap[MOVE_CONSTRAINT_DOWN]->setEnabled(anyColumn && validIdx && !isLast);
}

void TableWindow::setupDefShortcuts()
{
    // Widget context
    setShortcutContext({
                           REFRESH_STRUCTURE,
                           REFRESH_INDEXES,
                           REFRESH_TRIGGERS,
                           ADD_COLUMN,
                           EDIT_COLUMN,
                           DEL_COLUMN,
                           ADD_TABLE_CONSTRAINT,
                           EDIT_TABLE_CONSTRAINT,
                           DEL_TABLE_CONSTRAINT,
                           ADD_INDEX,
                           EDIT_INDEX,
                           DEL_INDEX,
                           ADD_TRIGGER,
                           EDIT_TRIGGER,
                           DEL_TRIGGER,
                       },
                       Qt::WidgetWithChildrenShortcut);

    defShortcut(REFRESH_STRUCTURE, Qt::Key_F5);
    defShortcut(ADD_COLUMN, Qt::Key_Insert);
    defShortcut(EDIT_COLUMN, Qt::Key_Return);
    defShortcut(DEL_COLUMN, Qt::Key_Delete);
    defShortcut(EXPORT, Qt::Key_E);
    defShortcut(IMPORT, Qt::Key_I);

    defShortcut(ADD_TABLE_CONSTRAINT, Qt::Key_Insert);
    defShortcut(EDIT_TABLE_CONSTRAINT, Qt::Key_Return);
    defShortcut(DEL_TABLE_CONSTRAINT, Qt::Key_Delete);

    defShortcut(REFRESH_INDEXES, Qt::Key_F5);
    defShortcut(ADD_INDEX, Qt::Key_Insert);
    defShortcut(EDIT_INDEX, Qt::Key_Return);
    defShortcut(DEL_INDEX, Qt::Key_Delete);

    defShortcut(REFRESH_TRIGGERS, Qt::Key_F5);
    defShortcut(ADD_TRIGGER, Qt::Key_Insert);
    defShortcut(EDIT_TRIGGER, Qt::Key_Return);
    defShortcut(DEL_TRIGGER, Qt::Key_Delete);

    defShortcut(NEXT_TAB, Qt::ALT + Qt::Key_Right);
    defShortcut(PREV_TAB, Qt::ALT + Qt::Key_Left);
}

void TableWindow::executionSuccessful()
{
    dataLoaded = true;
}

void TableWindow::executionFailed(const QString& errorText)
{
    notifyError(tr("Could not load data for table %1. Error details: %2").arg(table).arg(errorText));
}

void TableWindow::initDbAndTable()
{
    if (db->getVersion() == 2)
    {
        ui->withoutRowIdCheck->setVisible(false);
    }

    if (existingTable)
    {
        dataModel->setDb(db);
        dataModel->setDatabaseAndTable(database, table);
    }

    ui->tableNameEdit->setText(table); // TODO no attached/temp db name support here

    if (structureModel)
    {
        delete structureModel;
        structureModel = nullptr;
    }

    if (structureConstraintsModel)
    {
        delete structureConstraintsModel;
        structureConstraintsModel = nullptr;
    }

    if (constraintTabModel)
    {
        delete constraintTabModel;
        constraintTabModel = nullptr;
    }

    structureModel = new TableStructureModel(this);
    structureConstraintsModel = new TableConstraintsModel(this);
    constraintTabModel = new ConstraintTabModel(this);

    // Columns model signals
    connect(structureModel, SIGNAL(columnModified(QString,SqliteCreateTable::Column*)),
            structureConstraintsModel, SLOT(columnModified(QString,SqliteCreateTable::Column*)));
    connect(structureModel, SIGNAL(columnDeleted(QString)),
            structureConstraintsModel, SLOT(columnDeleted(QString)));
    connect(structureModel, SIGNAL(columnsOrderChanged()), this, SLOT(updateStructureToolbarState()));

    connect(structureModel, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(updateDdlTab()));
    connect(structureModel, SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)), this, SLOT(updateDdlTab()));
    connect(structureModel, SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(updateDdlTab()));
    connect(structureModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(updateDdlTab()));
    connect(structureModel, SIGNAL(modifiyStateChanged()), this, SIGNAL(modifyStatusChanged()));

    ui->structureView->setModel(structureModel);
    ui->structureView->verticalHeader()->setDefaultSectionSize(ui->structureView->fontMetrics().height() + 8);

    // Constraints model signals
    connect(structureConstraintsModel, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(updateDdlTab()));
    connect(structureConstraintsModel, SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)), this, SLOT(updateDdlTab()));
    connect(structureConstraintsModel, SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(updateDdlTab()));
    connect(structureConstraintsModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(updateDdlTab()));

    connect(structureConstraintsModel, SIGNAL(modifiyStateChanged()), this, SIGNAL(modifyStatusChanged()));
    connect(structureConstraintsModel, SIGNAL(constraintOrderChanged()), this, SLOT(updateTableConstraintsToolbarState()));

    ui->tableConstraintsView->setModel(structureConstraintsModel);
    ui->tableConstraintsView->verticalHeader()->setDefaultSectionSize(ui->tableConstraintsView->fontMetrics().height() + 8);

    // Constraint tab model signals
    connect(structureModel, SIGNAL(rowsInserted(QModelIndex,int,int)), constraintTabModel, SLOT(updateModel()));
    connect(structureModel, SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)), constraintTabModel, SLOT(updateModel()));
    connect(structureModel, SIGNAL(rowsRemoved(QModelIndex,int,int)), constraintTabModel, SLOT(updateModel()));
    connect(structureModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), constraintTabModel, SLOT(updateModel()));
    connect(structureConstraintsModel, SIGNAL(rowsInserted(QModelIndex,int,int)), constraintTabModel, SLOT(updateModel()));
    connect(structureConstraintsModel, SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)), constraintTabModel, SLOT(updateModel()));
    connect(structureConstraintsModel, SIGNAL(rowsRemoved(QModelIndex,int,int)), constraintTabModel, SLOT(updateModel()));
    connect(structureConstraintsModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), constraintTabModel, SLOT(updateModel()));

    ui->constraintsView->setModel(constraintTabModel);

    connect(ui->withoutRowIdCheck, SIGNAL(clicked()), this, SLOT(withOutRowIdChanged()));

    ui->ddlEdit->setSqliteVersion(db->getVersion());
    parseDdl();
    updateIndexes();
    updateTriggers();

    // (Re)connect to DB signals
    disconnect(this, SLOT(dbClosed()));
    connect(db, SIGNAL(disconnected()), this, SLOT(dbClosed()));

    // Selection model is recreated when setModel() is called on the view
    connect(ui->structureView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(updateStructureToolbarState()));
    connect(ui->tableConstraintsView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(updateTableConstraintsToolbarState()));
}

void TableWindow::setupCoverWidget()
{
    widgetCover = new WidgetCover(this);
    widgetCover->hide();

    coverCancelButton = new QPushButton();
    coverCancelButton->setText(tr("Interrupt"));
    coverCancelButton->setEnabled(false);

    coverBusyBar = new QProgressBar();
    coverBusyBar->setRange(0, 0);

    widgetCover->getContainerLayout()->addWidget(coverBusyBar, 0, 0);
    widgetCover->getContainerLayout()->addWidget(coverCancelButton, 1, 0);

    connect(coverCancelButton, SIGNAL(clicked()), this, SLOT(disableCoverCancelButton()));
    connect(coverCancelButton, SIGNAL(clicked()), structureExecutor, SLOT(interrupt()));
}

void TableWindow::parseDdl()
{
    if (existingTable)
    {
        SchemaResolver resolver(db);
        SqliteQueryPtr parsedObject = resolver.getParsedObject(database, table);
        if (!parsedObject.dynamicCast<SqliteCreateTable>())
        {
            notifyError(tr("Could not process the %1 table correctly. Unable to open a table window.").arg(table));
            invalid = true;
            return;
        }

        createTable = parsedObject.dynamicCast<SqliteCreateTable>();
    }
    else
    {
        createTable = SqliteCreateTablePtr::create();
        createTable->table = table;
        createTable->dialect = db->getDialect();
    }
    originalCreateTable = SqliteCreateTablePtr::create(*createTable);
    structureModel->setCreateTable(createTable.data());
    structureConstraintsModel->setCreateTable(createTable.data());
    constraintTabModel->setCreateTable(createTable.data());
    ui->withoutRowIdCheck->setChecked(!createTable->withOutRowId.isNull());
    ui->tableConstraintsView->resizeColumnsToContents();
    ui->structureView->resizeColumnsToContents();
    ui->constraintsView->resizeColumnsToContents();

    updateStructureToolbarState();
    updateTableConstraintsToolbarState();
    updateDdlTab();
}

void TableWindow::changeEvent(QEvent *e)
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

QVariant TableWindow::saveSession()
{
    QHash<QString,QVariant> sessionValue;
    sessionValue["table"] = table;
    sessionValue["db"] = db->getName();
    return sessionValue;
}

bool TableWindow::restoreSession(const QVariant& sessionValue)
{
    QHash<QString, QVariant> value = sessionValue.toHash();
    if (value.size() == 0)
    {
        notifyWarn("Could not restore window, because no database or table was stored in session for this window.");
        return false;
    }

    if (!value.contains("db") || !value.contains("table"))
    {
        notifyWarn("Could not restore window, because no database or table was stored in session for this window.");
        return false;
    }

    db = DBLIST->getByName(value["db"].toString());
    if (!db)
    {
        notifyWarn(tr("Could not restore window, because database %1 could not be resolved.").arg(value["db"].toString()));
        return false;
    }

    if (!db->isOpen())
        db->open();

    table = value["table"].toString();
    database = value["database"].toString();
    SchemaResolver resolver(db);
    if (!resolver.getTables(database).contains(table, Qt::CaseInsensitive))
    {
        notifyWarn(tr("Could not restore window, because the table %1 doesn't exist in the database %2.").arg(table).arg(db->getName()));
        return false;
    }

    initDbAndTable();
    applyInitialTab();
    return true;
}

Icon* TableWindow::getIconNameForMdiWindow()
{
    return ICONS.TABLE;
}

QString TableWindow::getTitleForMdiWindow()
{
    if (existingTable)
        return table + (!db ? "" : (" (" + db->getName() + ")"));

    QStringList existingNames = MainWindow::getInstance()->getMdiArea()->getWindowTitles();
    if (existingNames.contains(windowTitle()))
        return windowTitle();

    // Generate new name
    QString title = tr("New table %1").arg(newTableWindowNum++);
    while (existingNames.contains(title))
        title = tr("New table %1").arg(newTableWindowNum++);

    return title;
}

Db* TableWindow::getDb() const
{
    return db;
}

QString TableWindow::getTable() const
{
    return table;
}

void TableWindow::dbClosed()
{
    dataModel->setDb(nullptr);
    structureExecutor->setDb(nullptr);
    disconnect(this, SLOT(dbClosed()));
    getMdiWindow()->close();
}

void TableWindow::refreshStructure()
{
    parseDdl();
    updateIndexes();
    updateTriggers();
}

void TableWindow::commitStructure()
{
    if (!isModified())
    {
        qWarning() << "Called TableWindow::commitStructure(), but isModified() returned false.";
        updateStructureCommitState();
        return;
    }

    if (!validate())
        return;

    executeStructureChanges();
}

void TableWindow::changesSuccessfullyCommited()
{
    QStringList sqls = structureExecutor->getQueries();
    CFG->addDdlHistory(sqls.join(";\n"), db->getName(), db->getPath());

    widgetCover->hide();

    originalCreateTable = createTable;
    structureModel->setCreateTable(createTable.data());
    structureConstraintsModel->setCreateTable(createTable.data());

    QString oldTable = table;
    database = createTable->database;
    table = createTable->table;
    existingTable = true;
    initDbAndTable();
    updateStructureCommitState();
    updateNewTableState();
    updateWindowTitle();

    DBTREE->refreshSchema(db);

    if (tableModifier)
    {
        QList<QStringList> modifiedObjects = {
            tableModifier->getModifiedTables(),
            tableModifier->getModifiedIndexes(),
            tableModifier->getModifiedTriggers(),
            tableModifier->getModifiedViews()
        };
        NotifyManager* notifyManager = NotifyManager::getInstance();
        foreach (const QStringList& objList, modifiedObjects)
        {
            foreach (const QString& obj, objList)
            {
                if (obj.compare(oldTable, Qt::CaseInsensitive) == 0)
                    continue;

                notifyManager->modified(db, database, obj);
            }
        }
    }
}

void TableWindow::changesFailedToCommit(int errorCode, const QString& errorText)
{
    qDebug() << "TableWindow::changesFailedToCommit:" << errorCode << errorText;

    widgetCover->hide();
    notifyError(tr("Could not commit table structure. Error message: %1", "table window").arg(errorText));
}

void TableWindow::rollbackStructure()
{
    createTable = SqliteCreateTablePtr::create(*originalCreateTable.data());
    structureModel->setCreateTable(createTable.data());
    structureConstraintsModel->setCreateTable(createTable.data());
    constraintTabModel->setCreateTable(createTable.data());
    ui->tableNameEdit->setText(createTable->table);

    updateStructureCommitState();
    updateStructureToolbarState();
    updateTableConstraintsToolbarState();
}

void TableWindow::addColumn()
{
    SqliteCreateTable::Column column;
    column.setParent(createTable.data());

    ColumnDialog columnDialog(db, this);
    columnDialog.setColumn(&column);
    if (columnDialog.exec() != QDialog::Accepted)
        return;

    SqliteCreateTable::Column* modifiedColumn = columnDialog.getModifiedColumn();
    structureModel->appendColumn(modifiedColumn);
    ui->structureView->resizeColumnToContents(0);

    ui->structureView->setCurrentIndex(structureModel->index(structureModel->rowCount()-1, 0));
}

void TableWindow::editColumn()
{
    editColumn(structureCurrentIndex());
}

void TableWindow::delColumn()
{
    QModelIndex idx = structureCurrentIndex();
    delColumn(idx);
}

void TableWindow::moveColumnUp()
{
    QModelIndex idx = structureCurrentIndex();
    if (!idx.isValid())
    {
        qWarning() << "Called TableWindow::moveColumnUp() with invalid index.";
        return;
    }

    structureModel->moveColumnUp(idx.row());
}

void TableWindow::moveColumnDown()
{
    QModelIndex idx = structureCurrentIndex();
    if (!idx.isValid())
    {
        qWarning() << "Called TableWindow::moveColumnDown() with invalid index.";
        return;
    }

    structureModel->moveColumnDown(idx.row());
}


void TableWindow::addConstraint(ConstraintDialog::Constraint mode)
{
    NewConstraintDialog dialog(mode, createTable.data(), db, this);
    if (dialog.exec() != QDialog::Accepted)
        return;

    SqliteStatement* constrStmt = dialog.getConstraint();
    SqliteCreateTable::Constraint* tableConstr = dynamic_cast<SqliteCreateTable::Constraint*>(constrStmt);
    if (!tableConstr)
    {
        qCritical() << "Constraint returned from ConstraintDialog was not of table type, while we're trying to add table constraint.";
        return;
    }

    structureConstraintsModel->appendConstraint(tableConstr);
    ui->tableConstraintsView->resizeColumnToContents(0);
    ui->tableConstraintsView->resizeColumnToContents(1);
}

bool TableWindow::validate()
{
    if (ui->tableNameEdit->text().trimmed().isEmpty() && !blankNameWarningDisplayed)
    {
        notifyWarn(tr("A blank name for the table is allowed in SQLite, but it is not recommended. "
                               "Hit the commit button again to ignore this warning and proceed."));
        blankNameWarningDisplayed = true;
        return false;
    }

    if (structureModel->rowCount() == 0)
    {
        notifyError(tr("Cannot create a table without at least one column."));
        return false;
    }

    if (ui->withoutRowIdCheck->isChecked())
    {
        bool hasPk = false;
        bool isPkAutoIncr = false;

        if (createTable->getConstraints(SqliteCreateTable::Constraint::PRIMARY_KEY).size() > 0)
            hasPk = true;

        SqliteCreateTable::Column::Constraint* colConstraint;
        foreach (SqliteCreateTable::Column* column, createTable->columns)
        {
            colConstraint = column->getConstraint(SqliteCreateTable::Column::Constraint::PRIMARY_KEY);
            if (colConstraint)
            {
                hasPk = true;
                if (colConstraint->autoincrKw)
                    isPkAutoIncr = true;
            }
        }

        if (!hasPk)
        {
            notifyError(tr("Cannot create table without ROWID, if it has no PRIMARY KEY defined."
                           " Either uncheck the WITHOUT ROWID, or define a PRIMARY KEY."));
            return false;
        }

        if (isPkAutoIncr)
        {
            notifyError(tr("Cannot use AUTOINCREMENT for PRIMARY KEY when WITHOUT ROWID clause is used."
                           " Either uncheck the WITHOUT ROWID, or the AUTOINCREMENT in a PRIMARY KEY."));
            return false;
        }
    }

    return true;
}

bool TableWindow::isModified() const
{
    return (structureModel && structureModel->isModified()) ||
            (structureConstraintsModel && structureConstraintsModel->isModified()) ||
            (originalCreateTable &&
                (originalCreateTable->table != ui->tableNameEdit->text() ||
                 originalCreateTable->withOutRowId != createTable->withOutRowId)
            ) ||
            !existingTable;
}

TokenList TableWindow::indexColumnTokens(SqliteCreateIndexPtr index)
{
    if (index->indexedColumns.size() == 0)
        return TokenList();

    SqliteIndexedColumn* firstCol = index->indexedColumns.first();
    SqliteIndexedColumn* lastCol = index->indexedColumns.last();
    if (firstCol->tokens.size() == 0)
        return TokenList();

    if (lastCol->tokens.size() == 0)
        return TokenList();

    int firstIdx = index->tokens.indexOf(firstCol->tokens.first());
    int lastIdx = index->tokens.indexOf(lastCol->tokens.last());

    return index->tokens.mid(firstIdx, lastIdx-firstIdx+1);
}

QString TableWindow::getCurrentIndex() const
{
    int row = ui->indexList->currentRow();
    QTableWidgetItem* item = ui->indexList->item(row, 0);
    if (!item)
        return QString::null;

    return item->text();
}

QString TableWindow::getCurrentTrigger() const
{
    int row = ui->triggerList->currentRow();
    QTableWidgetItem* item = ui->triggerList->item(row, 0);
    if (!item)
        return QString::null;

    return item->text();
}

void TableWindow::applyInitialTab()
{
    if (existingTable && !table.isNull() && CFG_UI.General.OpenTablesOnData.get())
        ui->tabWidget->setCurrentIndex(1);
    else
        ui->tabWidget->setCurrentIndex(0);
}

void TableWindow::updateDdlTab()
{
    SqlFormatter* formatter = SQLITESTUDIO->getSqlFormatter();
    createTable->rebuildTokens();
    ui->ddlEdit->setPlainText(formatter->format(createTable));
}

void TableWindow::updateNewTableState()
{
    for (int i = 1; i < 5; i++)
        ui->tabWidget->setTabEnabled(i, existingTable);
}

void TableWindow::updateBlankNameWarningState()
{
    if (isModified())
        blankNameWarningDisplayed = false;
}

void TableWindow::addConstraint()
{
    addConstraint(ConstraintDialog::UNKNOWN);
}

void TableWindow::editConstraint()
{
    QModelIndex idx = ui->tableConstraintsView->currentIndex();
    editConstraint(idx);
}

void TableWindow::delConstraint()
{
    QModelIndex idx = ui->tableConstraintsView->currentIndex();
    delConstraint(idx);
}

void TableWindow::editConstraint(const QModelIndex& idx)
{
    if (!idx.isValid())
        return;

    SqliteCreateTable::Constraint* constr = structureConstraintsModel->getConstraint(idx.row());
    ConstraintDialog dialog(ConstraintDialog::EDIT, constr, createTable.data(), db, this);
    if (dialog.exec() != QDialog::Accepted)
        return;

    structureConstraintsModel->constraintModified(idx.row());
    ui->tableConstraintsView->resizeColumnToContents(0);
    ui->tableConstraintsView->resizeColumnToContents(1);
}

void TableWindow::delConstraint(const QModelIndex& idx)
{
    if (!idx.isValid())
        return;

    SqliteCreateTable::Constraint* constr = structureConstraintsModel->getConstraint(idx.row());

    QString arg = constr->name.isNull() ? constr->typeString() : constr->name;
    QString msg = tr("Are you sure you want to delete table constraint '%1'?", "table window").arg(arg);
    int btn = QMessageBox::question(this, tr("Delete constraint", "table window"), msg);
    if (btn != QMessageBox::Yes)
        return;

    structureConstraintsModel->delConstraint(idx.row());
    ui->structureView->resizeColumnToContents(0);
}

void TableWindow::moveConstraintUp()
{
    QModelIndex idx = ui->tableConstraintsView->currentIndex();
    if (!idx.isValid())
        return;

    structureConstraintsModel->moveConstraintUp(idx.row());
    updateTableConstraintsToolbarState();
    updateStructureCommitState();
}

void TableWindow::moveConstraintDown()
{
    QModelIndex idx = ui->tableConstraintsView->currentIndex();
    if (!idx.isValid())
        return;

    structureConstraintsModel->moveConstraintDown(idx.row());
    updateTableConstraintsToolbarState();
    updateStructureCommitState();
}

void TableWindow::addPk()
{
    addConstraint(ConstraintDialog::PK);
}

void TableWindow::addFk()
{
    addConstraint(ConstraintDialog::FK);
}

void TableWindow::addUnique()
{
    addConstraint(ConstraintDialog::UNIQUE);
}

void TableWindow::addCheck()
{
    addConstraint(ConstraintDialog::CHECK);
}

void TableWindow::exportTable()
{
    if (!ExportManager::isAnyPluginAvailable())
    {
        notifyError(tr("Cannot export, because no export plugin is loaded."));
        return;
    }

    ExportDialog dialog(this);
    dialog.setTableMode(db, table);
    dialog.exec();
}

void TableWindow::importTable()
{
    // TODO
}

void TableWindow::populateTable()
{
    // TODO
}

void TableWindow::createSimilarTable()
{
    // TODO
}

void TableWindow::tabChanged(int newTab)
{
    switch (newTab)
    {
        case 1:
        {
            if (!dataLoaded)
                ui->dataView->refreshData();

            break;
        }
    }
}

void TableWindow::on_structureView_doubleClicked(const QModelIndex &index)
{
    editColumn(index);
}

void TableWindow::on_tableConstraintsView_doubleClicked(const QModelIndex &index)
{
    editConstraint(index);
}

void TableWindow::disableCoverCancelButton()
{
    coverCancelButton->setEnabled(false);
}

void TableWindow::nameChanged()
{
    if (!createTable)
        return;

    createTable->table = ui->tableNameEdit->text();
    updateDdlTab();
}

void TableWindow::withOutRowIdChanged()
{
    if (!createTable)
        return;

    createTable->withOutRowId = ui->withoutRowIdCheck->isChecked() ? QStringLiteral("ROWID") : QString::null;
    updateDdlTab();
    emit modifyStatusChanged();
}

void TableWindow::addIndex()
{
    DbObjectDialogs dialogs(db, this);
    dialogs.addIndex(table);
    updateIndexes();
}

void TableWindow::editIndex()
{
    QString index = getCurrentIndex();
    if (index.isNull())
        return;

    DbObjectDialogs dialogs(db, this);
    dialogs.editIndex(index);
    updateIndexes();
}

void TableWindow::delIndex()
{
    QString index = getCurrentIndex();
    if (index.isNull())
        return;

    DbObjectDialogs dialogs(db, this);
    dialogs.dropObject(index);
    updateIndexes();
}

void TableWindow::addTrigger()
{
    DbObjectDialogs dialogs(db, this);
    dialogs.addTriggerOnTable(table);
    updateTriggers();
}

void TableWindow::editTrigger()
{
    QString trigger = getCurrentTrigger();
    if (trigger.isNull())
        return;

    DbObjectDialogs dialogs(db, this);
    dialogs.editTrigger(trigger);
    updateTriggers();
}

void TableWindow::delTrigger()
{
    QString trigger = getCurrentTrigger();
    if (trigger.isNull())
        return;

    DbObjectDialogs dialogs(db, this);
    dialogs.dropObject(trigger);
    updateTriggers();
}

void TableWindow::updateIndexesState()
{
    bool editDel = ui->indexList->currentItem() != nullptr;
    actionMap[REFRESH_INDEXES]->setEnabled(existingTable);
    actionMap[ADD_INDEX]->setEnabled(existingTable);
    actionMap[EDIT_INDEX]->setEnabled(editDel);
    actionMap[DEL_INDEX]->setEnabled(editDel);
}

void TableWindow::updateTriggersState()
{
    bool editDel = ui->triggerList->currentItem() != nullptr;
    actionMap[REFRESH_TRIGGERS]->setEnabled(existingTable);
    actionMap[ADD_TRIGGER]->setEnabled(existingTable);
    actionMap[EDIT_TRIGGER]->setEnabled(editDel);
    actionMap[DEL_TRIGGER]->setEnabled(editDel);
}

void TableWindow::nextTab()
{
    int idx = ui->tabWidget->currentIndex();
    idx++;
    ui->tabWidget->setCurrentIndex(idx);
}

void TableWindow::prevTab()
{
    int idx = ui->tabWidget->currentIndex();
    idx--;
    ui->tabWidget->setCurrentIndex(idx);
}

void TableWindow::updateIndexes()
{
    ui->indexList->clear();

    if (!db)
        return;

    SchemaResolver resolver(db);
    resolver.setIgnoreSystemObjects(true);
    QList<SqliteCreateIndexPtr> indexes = resolver.getParsedIndexesForTable(database, table);

    ui->indexList->setColumnCount(4);
    ui->indexList->setRowCount(indexes.size());
    ui->indexList->setHorizontalHeaderLabels({
                                                 tr("Name", "table window indexes"),
                                                 tr("Unique", "table window indexes"),
                                                 tr("Columns", "table window indexes"),
                                                 tr("Partial index condition", "table window indexes"),
                                             });

    Dialect dialect= db->getDialect();
    if (dialect == Dialect::Sqlite2)
        ui->indexList->setColumnCount(3);

    ui->indexList->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);

    QTableWidgetItem* item;
    int row = 0;
    foreach (SqliteCreateIndexPtr index, indexes)
    {
        item = new QTableWidgetItem(index->index);
        item->setFlags(Qt::ItemIsEnabled|Qt::ItemIsSelectable);
        ui->indexList->setItem(row, 0, item);

        // TODO a deletate to make the checkbox in the center, or use setCellWidget()
        item = new QTableWidgetItem();
        item->setFlags(Qt::ItemIsEnabled|Qt::ItemIsSelectable);
        item->setCheckState(index->uniqueKw ? Qt::Checked : Qt::Unchecked);
        ui->indexList->setItem(row, 1, item);

        item = new QTableWidgetItem(indexColumnTokens(index).detokenize());
        item->setFlags(Qt::ItemIsEnabled|Qt::ItemIsSelectable);
        ui->indexList->setItem(row, 2, item);

        if (dialect == Dialect::Sqlite3)
        {
            item = new QTableWidgetItem(index->where ? index->where->detokenize() : "");
            item->setFlags(Qt::ItemIsEnabled|Qt::ItemIsSelectable);
            ui->indexList->setItem(row, 3, item);
        }

        row++;
    }

    ui->indexList->resizeColumnsToContents();
    updateIndexesState();
}

void TableWindow::updateTriggers()
{
    if (!db)
        return;

    SchemaResolver resolver(db);
    QList<SqliteCreateTriggerPtr> triggers = resolver.getParsedTriggersForTable(database, table);

    ui->triggerList->setColumnCount(4);
    ui->triggerList->setRowCount(triggers.size());
    ui->triggerList->horizontalHeader()->setMaximumSectionSize(200);
    ui->triggerList->setHorizontalHeaderLabels({
                                                 tr("Name", "table window triggers"),
                                                 tr("Event", "table window triggers"),
                                                 tr("Condition", "table window triggers"),
                                                 tr("Details", "table window triggers")
                                             });

    QTableWidgetItem* item;
    QString timeAndEvent;
    int row = 0;
    foreach (SqliteCreateTriggerPtr trig, triggers)
    {
        item = new QTableWidgetItem(trig->trigger);
        item->setFlags(Qt::ItemIsEnabled|Qt::ItemIsSelectable);
        ui->triggerList->setItem(row, 0, item);

        timeAndEvent = trig->tokensMap["trigger_time"].detokenize() + trig->tokensMap["trigger_event"].detokenize();
        item = new QTableWidgetItem(timeAndEvent);
        item->setFlags(Qt::ItemIsEnabled|Qt::ItemIsSelectable);
        ui->triggerList->setItem(row, 1, item);

        item = new QTableWidgetItem(trig->precondition ? trig->precondition->detokenize().trimmed() : "");
        item->setFlags(Qt::ItemIsEnabled|Qt::ItemIsSelectable);
        ui->triggerList->setItem(row, 2, item);

        item = new QTableWidgetItem(trig->tokensMap["trigger_cmd_list"].detokenize().trimmed());
        item->setFlags(Qt::ItemIsEnabled|Qt::ItemIsSelectable);
        ui->triggerList->setItem(row, 3, item);

        row++;
    }

    ui->triggerList->resizeColumnsToContents();
    updateTriggersState();
}

void TableWindow::editColumn(const QString& columnName)
{
    QModelIndex colIdx = structureModel->findColumn(columnName);
    if (!colIdx.isValid())
        return;

    editColumn(colIdx);
}

bool TableWindow::restoreSessionNextTime()
{
    return existingTable;
}

void TableWindow::resizeEvent(QResizeEvent* e)
{
    MdiChild::resizeEvent(e);
    widgetCover->widgetResized();
}
