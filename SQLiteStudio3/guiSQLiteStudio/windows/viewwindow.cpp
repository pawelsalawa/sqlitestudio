#include "viewwindow.h"
#include "ui_viewwindow.h"
#include "common/unused.h"
#include "schemaresolver.h"
#include "services/notifymanager.h"
#include "services/dbmanager.h"
#include "mainwindow.h"
#include "mdiarea.h"
#include "datagrid/sqlquerymodel.h"
#include "common/utils_sql.h"
#include "viewmodifier.h"
#include "common/widgetcover.h"
#include "db/chainexecutor.h"
#include "dbtree/dbtree.h"
#include "parser/ast/sqlitecreatetrigger.h"
#include "dialogs/messagelistdialog.h"
#include "dbobjectdialogs.h"
#include "dialogs/ddlpreviewdialog.h"
#include "uiconfig.h"
#include "services/config.h"
#include "services/codeformatter.h"
#include "themetuner.h"
#include "datagrid/sqlviewmodel.h"
#include <QPushButton>
#include <QProgressBar>
#include <QDebug>
#include <QMessageBox>
#include <QCheckBox>

CFG_KEYS_DEFINE(ViewWindow)

ViewWindow::ViewWindow(QWidget *parent) :
    MdiChild(parent),
    ui(new Ui::ViewWindow)
{
    init();
    applyInitialTab();
}

ViewWindow::ViewWindow(Db* db, QWidget* parent) :
    MdiChild(parent),
    db(db),
    ui(new Ui::ViewWindow)
{
    newView();
    init();
    ui->dbCombo->setCurrentDb(db);
    applyInitialTab();
}

ViewWindow::ViewWindow(const ViewWindow& win) :
    MdiChild(win.parentWidget()),
    db(win.db),
    database(win.database),
    view(win.view),
    ui(new Ui::ViewWindow)
{
    init();
    initView();
    applyInitialTab();
}

ViewWindow::ViewWindow(QWidget* parent, Db* db, const QString& database, const QString& view) :
    MdiChild(parent),
    db(db),
    database(database),
    view(view),
    ui(new Ui::ViewWindow)
{
    init();
    initView();
    applyInitialTab();
}

ViewWindow::~ViewWindow()
{
    delete ui;
}

void ViewWindow::changeEvent(QEvent *e)
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

QVariant ViewWindow::saveSession()
{
    if (!db || DBLIST->isTemporary(db))
        return QVariant();

    QHash<QString,QVariant> sessionValue;
    sessionValue["view"] = view;
    sessionValue["db"] = db->getName();
    return sessionValue;
}

bool ViewWindow::restoreSession(const QVariant& sessionValue)
{
    QHash<QString, QVariant> value = sessionValue.toHash();
    if (value.size() == 0)
    {
        notifyWarn(tr("Could not restore window '%1', because no database or view was stored in session for this window.").arg(value["title"].toString()));
        return false;
    }

    if (!value.contains("db") || !value.contains("view"))
    {
        notifyWarn(tr("Could not restore window '%1', because no database or view was stored in session for this window.").arg(value["title"].toString()));
        return false;
    }

    db = DBLIST->getByName(value["db"].toString());
    if (!db)
    {
        notifyWarn(tr("Could not restore window '%1', because database %2 could not be resolved.").arg(value["title"].toString(), value["db"].toString()));
        return false;
    }

    if (!db->isOpen() && !db->open())
    {
        notifyWarn(tr("Could not restore window '%1', because database %2 could not be open.").arg(value["title"].toString(), value["db"].toString()));
        return false;
    }

    view = value["view"].toString();
    database = value["database"].toString();
    SchemaResolver resolver(db);
    if (!resolver.getViews(database).contains(view, Qt::CaseInsensitive))
    {
        notifyWarn(tr("Could not restore window '%1', because the view %2 doesn't exist in the database %3.").arg(value["title"].toString(), view, db->getName()));
        return false;
    }

    initView();
    applyInitialTab();
    return true;
}

Icon* ViewWindow::getIconNameForMdiWindow()
{
    return ICONS.VIEW;
}

QString ViewWindow::getTitleForMdiWindow()
{
    QString dbSuffix = (!db ? "" : (" (" + db->getName() + ")"));
    if (existingView)
        return view + dbSuffix;

    QStringList existingNames = MDIAREA->getWindowTitles();
    if (existingNames.contains(windowTitle()))
        return windowTitle();

    // Generate new name
    QString title = tr("New view %1").arg(newViewWindowNum++);
    while (existingNames.contains(title))
        title = tr("New view %1").arg(newViewWindowNum++);

    title += dbSuffix;
    return title;
}

void ViewWindow::createActions()
{
    createQueryTabActions();
    createTriggersTabActions();

    createAction(NEXT_TAB, "next tab", this, SLOT(nextTab()), this);
    createAction(PREV_TAB, "prev tab", this, SLOT(prevTab()), this);
}

void ViewWindow::setupDefShortcuts()
{
    // Widget context
    setShortcutContext({
                           COMMIT_QUERY,
                           ROLLBACK_QUERY,
                           REFRESH_TRIGGERS,
                           ADD_TRIGGER,
                           EDIT_TRIGGER,
                           DEL_TRIGGER,
                           EXECUTE_QUERY
                       },
                       Qt::WidgetWithChildrenShortcut);

    BIND_SHORTCUTS(ViewWindow, Action);
}

bool ViewWindow::restoreSessionNextTime()
{
    return existingView && db && !DBLIST->isTemporary(db);
}

QToolBar* ViewWindow::getToolBar(int toolbar) const
{
    switch (static_cast<ToolBar>(toolbar))
    {
        case TOOLBAR_QUERY:
            return ui->queryToolbar;
        case TOOLBAR_TRIGGERS:
            return ui->triggersToolbar;
    }
    return nullptr;
}

void ViewWindow::init()
{
    ui->setupUi(this);

    THEME_TUNER->manageCompactLayout({
                                      ui->queryTab,
                                      ui->dataTab,
                                      ui->triggersTab,
                                      ui->ddlTab
                                     });

    dataModel = new SqlViewModel(this);
    ui->dataView->init(dataModel);

    ui->queryEdit->setVirtualSqlExpression("CREATE VIEW name AS %1");
    ui->queryEdit->setDb(db);
    ui->queryEdit->setOpenSaveActionsEnabled(false);

    connect(dataModel, SIGNAL(executionSuccessful()), this, SLOT(executionSuccessful()));
    connect(dataModel, SIGNAL(executionFailed(QString)), this, SLOT(executionFailed(QString)));
    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));
    connect(ui->nameEdit, SIGNAL(textChanged(QString)), this, SLOT(updateQueryToolbarStatus()));
    connect(ui->queryEdit, SIGNAL(textChanged()), this, SLOT(updateQueryToolbarStatus()));
    connect(ui->queryEdit, SIGNAL(errorsChecked(bool)), this, SLOT(updateQueryToolbarStatus()));
    connect(ui->triggersList, SIGNAL(itemSelectionChanged()), this, SLOT(updateTriggersState()));
    connect(ui->triggersList, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(triggerViewDoubleClicked(QModelIndex)));
    connect(ui->outputColumnsTable, SIGNAL(currentRowChanged(int)), this, SLOT(updateColumnButtons()));
    connect(ui->outputColumnsTable->model(), SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)), this, SLOT(updateColumnButtons()));
    connect(ui->outputColumnsTable->model(), SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)), this, SLOT(updateQueryToolbarStatus()));
    connect(ui->outputColumnsTable, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(updateQueryToolbarStatus()));
    connect(CFG_UI.General.DataTabAsFirstInViews, SIGNAL(changed(QVariant)), this, SLOT(updateTabsOrder()));
    connect(CFG_UI.Fonts.DataView, SIGNAL(changed(QVariant)), this, SLOT(updateFont()));
    connect(NotifyManager::getInstance(), SIGNAL(objectModified(Db*,QString,QString)), this, SLOT(handleObjectModified(Db*,QString,QString)));

    structureExecutor = new ChainExecutor(this);
    connect(structureExecutor, SIGNAL(success(SqlQueryPtr)), this, SLOT(changesSuccessfullyCommitted()));
    connect(structureExecutor, SIGNAL(failure(int,QString)), this, SLOT(changesFailedToCommit(int,QString)));

    setupCoverWidget();

    initActions();

    ui->splitter->setStretchFactor(0, 1);
    ui->splitter->setStretchFactor(1, 3);

    updateOutputColumnsVisibility();

    updateTabsOrder();
    createDbCombo();

    updateFont();
    refreshTriggers();
    updateQueryToolbarStatus();
    updateTriggersState();
    updateColumnButtons();
    updateAfterInit();
}

void ViewWindow::updateAfterInit()
{
    for (QWidget* tab : {ui->dataTab, ui->triggersTab})
        ui->tabWidget->setTabEnabled(ui->tabWidget->indexOf(tab), existingView);
}

void ViewWindow::createDbCombo()
{
    ui->dbCombo->setFixedWidth(100);
    ui->dbCombo->setToolTip(tr("Database"));
    connect(ui->dbCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(dbChanged()));
}

void ViewWindow::newView()
{
    existingView = false;
    view = "";
}

void ViewWindow::initView()
{
    ui->nameEdit->setText(view);

    parseDdl();

    if (!createView)
        return; // error occured while parsing ddl, window will be closed

    ui->dbCombo->setCurrentDb(db);
    if (existingView)
    {
        dataModel->setDb(db);
        dataModel->setQuery(originalCreateView->equivalentSelectTokens().detokenize());
        dataModel->setDatabaseAndView(database, view);
        ui->dbCombo->setDisabled(true);
    }
    ui->queryEdit->setDb(db);

    ui->queryEdit->setPlainText(createView->select->detokenize());

    if (createView->columns.size() > 0)
    {
        columnsFromViewToList();
        outputColumnsCheck->setChecked(true);
    }

    updateDdlTab();

    refreshTriggers();

    // Disconnect first in case this method is (re)executed upon dependant table changes.
    disconnect(db, SIGNAL(dbObjectDeleted(QString,QString,DbObjectType)), this, SLOT(checkIfViewDeleted(QString,QString,DbObjectType)));
    connect(db, SIGNAL(dbObjectDeleted(QString,QString,DbObjectType)), this, SLOT(checkIfViewDeleted(QString,QString,DbObjectType)));
}

void ViewWindow::setupCoverWidget()
{
    widgetCover = new WidgetCover(this);
    widgetCover->hide();
    connect(widgetCover, SIGNAL(cancelClicked()), structureExecutor, SLOT(interrupt()));
}

void ViewWindow::createQueryTabActions()
{
    createAction(REFRESH_QUERY, ICONS.RELOAD, tr("Refresh the view", "view window"), this, SLOT(refreshView()), ui->queryToolbar, ui->queryEdit);
    ui->queryToolbar->addSeparator();
    createAction(COMMIT_QUERY, ICONS.COMMIT, tr("Commit the view changes", "view window"), this, SLOT(commitView()), ui->queryToolbar, ui->queryEdit);
    createAction(ROLLBACK_QUERY, ICONS.ROLLBACK, tr("Rollback the view changes", "view window"), this, SLOT(rollbackView()), ui->queryToolbar, ui->queryEdit);
    ui->queryToolbar->addSeparator();
    ui->queryToolbar->addAction(ui->queryEdit->getAction(SqlEditor::FORMAT_SQL));

    outputColumnsCheck = new QAction(ICONS.COLUMNS, tr("Explicit column names"), this);
    outputColumnsCheck->setCheckable(true);
    connect(outputColumnsCheck, SIGNAL(toggled(bool)), this, SLOT(updateOutputColumnsVisibility()));

    outputColumnsSeparator = ui->queryToolbar->addSeparator();
    ui->queryToolbar->addAction(outputColumnsCheck);
    createAction(GENERATE_OUTPUT_COLUMNS, ICONS.GENERATE_COLUMNS, tr("Generate output column names automatically basing on result columns of the view."), this, SLOT(generateOutputColumns()), ui->queryToolbar);
    createAction(ADD_COLUMN, ICONS.TABLE_COLUMN_ADD, tr("Add column", "view window"), this, SLOT(addColumn()), ui->queryToolbar);
    createAction(EDIT_COLUMN, ICONS.TABLE_COLUMN_EDIT, tr("Edit column", "view window"), this, SLOT(editColumn()), ui->queryToolbar);
    createAction(DEL_COLUMN, ICONS.TABLE_COLUMN_DELETE, tr("Delete column", "view window"), this, SLOT(delColumn()), ui->queryToolbar);
    createAction(MOVE_COLUMN_UP, ICONS.MOVE_UP, tr("Move column up", "view window"), this, SLOT(moveColumnUp()), ui->queryToolbar);
    createAction(MOVE_COLUMN_DOWN, ICONS.MOVE_DOWN, tr("Move column down", "view window"), this, SLOT(moveColumnDown()), ui->queryToolbar);
    createAction(EXECUTE_QUERY, QString(), this, SLOT(executeQuery()), this);
}

void ViewWindow::createTriggersTabActions()
{
    createAction(REFRESH_TRIGGERS, ICONS.RELOAD, tr("Refresh trigger list", "view window"), this, SLOT(refreshTriggers()), ui->triggersToolbar, ui->triggersList);
    ui->triggersToolbar->addSeparator();
    createAction(ADD_TRIGGER, ICONS.TRIGGER_ADD, tr("Create new trigger", "view window"), this, SLOT(addTrigger()), ui->triggersToolbar, ui->triggersList);
    createAction(EDIT_TRIGGER, ICONS.TRIGGER_EDIT, tr("Edit selected trigger", "view window"), this, SLOT(editTrigger()), ui->triggersToolbar, ui->triggersList);
    createAction(DEL_TRIGGER, ICONS.TRIGGER_DEL, tr("Delete selected trigger", "view window"), this, SLOT(deleteTrigger()), ui->triggersToolbar, ui->triggersList);
}
QString ViewWindow::getView() const
{
    return view;
}

void ViewWindow::setSelect(const QString &selectSql)
{
    ui->queryEdit->setPlainText(selectSql);
}

bool ViewWindow::isUncommitted() const
{
    return ui->dataView->isUncommitted() || isModified();
}

QString ViewWindow::getQuitUncommittedConfirmMessage() const
{
    QString title = getMdiWindow()->windowTitle();
    if (ui->dataView->isUncommitted() && isModified())
        return tr("View window \"%1\" has uncommitted structure modifications and data.").arg(title);
    else if (ui->dataView->isUncommitted())
        return tr("View window \"%1\" has uncommitted data.").arg(title);
    else if (isModified())
        return tr("View window \"%1\" has uncommitted structure modifications.").arg(title);
    else
    {
        qCritical() << "Unhandled message case in ViewWindow::getQuitUncommittedConfirmMessage().";
        return QString();
    }
}

Db* ViewWindow::getAssociatedDb() const
{
    return db;
}

void ViewWindow::staticInit()
{
    qRegisterMetaType<ViewWindow>("ViewWindow");
}

void ViewWindow::insertAction(ExtActionPrototype* action, ViewWindow::ToolBar toolbar)
{
    return ExtActionContainer::insertAction<ViewWindow>(action, toolbar);
}

void ViewWindow::insertActionBefore(ExtActionPrototype* action, ViewWindow::Action beforeAction, ViewWindow::ToolBar toolbar)
{
    return ExtActionContainer::insertActionBefore<ViewWindow>(action, beforeAction, toolbar);
}

void ViewWindow::insertActionAfter(ExtActionPrototype* action, ViewWindow::Action afterAction, ViewWindow::ToolBar toolbar)
{
    return ExtActionContainer::insertActionAfter<ViewWindow>(action, afterAction, toolbar);
}

void ViewWindow::removeAction(ExtActionPrototype* action, ViewWindow::ToolBar toolbar)
{
    ExtActionContainer::removeAction<ViewWindow>(action, toolbar);
}

QString ViewWindow::getDatabase() const
{
    return database;
}

Db* ViewWindow::getDb() const
{
    return db;
}

void ViewWindow::refreshView()
{
    initView();
    updateTriggersState();
}

void ViewWindow::executeQuery()
{
    if (isModified())
    {
        if (!actionMap[COMMIT_QUERY]->isEnabled())
            return;

        commitView(false, true);
        return;
    }

    switchToDataAndReload();
}

void ViewWindow::commitView(bool skipWarnings, bool loadDataAfterNextCommit)
{
    this->loadDataAfterNextCommit = loadDataAfterNextCommit;
    if (!isModified())
    {
        qWarning() << "Called ViewWindow::commitView(), but isModified() returned false.";
        updateQueryToolbarStatus();
        return;
    }

    if (!validate(skipWarnings))
        return;

    executeStructureChanges();
}

void ViewWindow::rollbackView()
{
    createView = SqliteCreateViewPtr::create(*originalCreateView.data());
    ui->nameEdit->setText(createView->view);
    ui->queryEdit->setPlainText(createView->select->detokenize());

    columnsFromViewToList();
    updateQueryToolbarStatus();
    updateDdlTab();
}

QString ViewWindow::getCurrentTrigger() const
{
    int row = ui->triggersList->currentRow();
    QTableWidgetItem* item = ui->triggersList->item(row, 0);
    if (!item)
        return QString();

    return item->text();
}

void ViewWindow::applyInitialTab()
{
    if (existingView && !view.isNull() && CFG_UI.General.OpenViewsOnData.get())
        ui->tabWidget->setCurrentIndex(getDataTabIdx());
    else
        ui->tabWidget->setCurrentIndex(getQueryTabIdx());
}

QString ViewWindow::getCurrentDdl() const
{
    static_qstring(ddlTpl, "CREATE VIEW %1%2 AS %3");
    QString columnsStr = "";
    if (outputColumnsCheck->isChecked() && ui->outputColumnsTable->count() > 0)
        columnsStr = "(" + collectColumnNames().join(", ") + ")";

    return ddlTpl.arg(
                    wrapObjIfNeeded(ui->nameEdit->text()),
                    columnsStr,
                    ui->queryEdit->toPlainText()
                );
}

QStringList ViewWindow::indexedColumnsToNamesOnly(const QList<SqliteIndexedColumn*>& columns) const
{
    QStringList names;
    for (SqliteIndexedColumn* col : columns)
        names << col->name;

    return names;
}

QStringList ViewWindow::collectColumnNames() const
{
    QStringList cols;
    for (int row = 0; row < ui->outputColumnsTable->count(); row++)
        cols << wrapObjIfNeeded(ui->outputColumnsTable->item(row)->text());

    return cols;
}

void ViewWindow::columnsFromViewToList()
{
    ui->outputColumnsTable->clear();
    ui->outputColumnsTable->addItems(indexedColumnsToNamesOnly(createView->columns));

    QListWidgetItem* item = nullptr;
    for (int row = 0; row < ui->outputColumnsTable->count(); row++)
    {
        item = ui->outputColumnsTable->item(row);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
    }
}

int ViewWindow::getDataTabIdx() const
{
    return ui->tabWidget->indexOf(ui->dataTab);
}

int ViewWindow::getQueryTabIdx() const
{
    return ui->tabWidget->indexOf(ui->queryTab);
}

int ViewWindow::getDdlTabIdx() const
{
    return ui->tabWidget->indexOf(ui->ddlTab);
}

void ViewWindow::switchToDataAndReload()
{
    ui->tabWidget->setCurrentWidget(ui->dataTab);
    // Query execution will happen automatically upon changing tab to data tab.
}

void ViewWindow::addTrigger()
{
    DbObjectDialogs dialogs(db, this);
    dialogs.addTriggerOnView(view);
    refreshTriggers();
}

void ViewWindow::editTrigger()
{
    QString trigger = getCurrentTrigger();
    if (trigger.isNull())
        return;

    DbObjectDialogs dialogs(db, this);
    dialogs.editTrigger(trigger);
    refreshTriggers();
}

void ViewWindow::deleteTrigger()
{
    QString trigger = getCurrentTrigger();
    if (trigger.isNull())
        return;

    DbObjectDialogs dialogs(db, this);
    dialogs.dropObject(DbObjectDialogs::Type::TRIGGER, trigger);
    refreshTriggers();
}

void ViewWindow::executionSuccessful()
{
    modifyingThisView = false;
    dataLoaded = true;
}

void ViewWindow::executionFailed(const QString& errorMessage)
{
    modifyingThisView = false;
    notifyError(tr("Could not load data for view %1. Error details: %2").arg(view, errorMessage));
}

void ViewWindow::tabChanged(int tabIdx)
{
    if (tabsMoving)
        return;

    if (tabIdx == getDataTabIdx())
    {
            if (isModified())
            {
                QMessageBox box(QMessageBox::Question, tr("Uncommitted changes"),
                                tr("There are uncommitted structure modifications."),
                                QMessageBox::NoButton, this);
                box.setInformativeText(tr("You cannot browse or edit data until you have "
                                          "the view structure settled.\n"
                                          "Do you want to commit the structure, or do you want to go back to the structure tab?"));
                box.addButton(tr("Go back to structure tab"), QMessageBox::RejectRole);
                QAbstractButton* commitButton = box.addButton(tr("Commit modifications and browse data"),
                                                              QMessageBox::ApplyRole);
                box.exec();

                ui->tabWidget->setCurrentIndex(0);
                if (box.clickedButton() == commitButton)
                    commitView(true);

                return;
            }

            if (!dataLoaded)
                ui->dataView->refreshData();

            return;
    }

    if (tabIdx == getDdlTabIdx())
    {
        updateDdlTab();
        return;
    }
}

void ViewWindow::updateQueryToolbarStatus()
{
    bool modified = isModified();
    bool queryOk = ui->queryEdit->isSyntaxChecked() && !ui->queryEdit->haveErrors();
    bool dbOk = ui->dbCombo->currentIndex() > -1;
    actionMap[COMMIT_QUERY]->setEnabled(modified && queryOk && dbOk);
    actionMap[ROLLBACK_QUERY]->setEnabled(modified && existingView);
    actionMap[REFRESH_QUERY]->setEnabled(existingView);
}

void ViewWindow::changesSuccessfullyCommitted()
{
    QStringList sqls = structureExecutor->getQueries();
    CFG->addDdlHistory(sqls.join("\n"), db->getName(), db->getPath());

    widgetCover->hide();

    originalCreateView = createView;
    dataLoaded = false;

    //QString oldView = view; // uncomment when implementing notify manager call
    database = createView->database;
    QString oldView = view;
    view = createView->view;

    emit sessionValueChanged();

    if (!existingView)
        notifyInfo(tr("View '%1' was committed successfully.").arg(view));
    else if (oldView.compare(view, Qt::CaseInsensitive) == 0)
        notifyInfo(tr("Committed changes for view '%1' successfully.").arg(view));
    else
        notifyInfo(tr("Committed changes for view '%1' (named before '%2') successfully.").arg(view, oldView));

    existingView = true;
    initView();
    updateQueryToolbarStatus();
    updateWindowTitle();
    updateAfterInit();

    DBTREE->refreshSchema(db);

    if (loadDataAfterNextCommit)
    {
        loadDataAfterNextCommit = false;
        switchToDataAndReload();
    }
}

void ViewWindow::changesFailedToCommit(int errorCode, const QString& errorText)
{
    qDebug() << "ViewWindow::changesFailedToCommit:" << errorCode << errorText;

    widgetCover->hide();

    NotifyManager::getInstance()->error(tr("Could not commit view changes. Error message: %1", "view window").arg(errorText));
}

void ViewWindow::updateTriggersState()
{
    bool editDel = ui->triggersList->currentItem() != nullptr;
    actionMap[REFRESH_TRIGGERS]->setEnabled(existingView);
    actionMap[ADD_TRIGGER]->setEnabled(existingView);
    actionMap[EDIT_TRIGGER]->setEnabled(editDel);
    actionMap[DEL_TRIGGER]->setEnabled(editDel);
}

void ViewWindow::nextTab()
{
    int idx = ui->tabWidget->currentIndex();
    idx++;
    ui->tabWidget->setCurrentIndex(idx);
}

void ViewWindow::prevTab()
{
    int idx = ui->tabWidget->currentIndex();
    idx--;
    ui->tabWidget->setCurrentIndex(idx);
}

void ViewWindow::dbClosedFinalCleanup()
{
    db = nullptr;
    dataModel->setDb(nullptr);
    ui->queryEdit->setDb(nullptr);
    structureExecutor->setDb(nullptr);
}

void ViewWindow::checkIfViewDeleted(const QString& database, const QString& object, DbObjectType type)
{
    UNUSED(database);

    if (type == DbObjectType::TRIGGER)
    {
        for (int i = 0, total = ui->triggersList->rowCount(); i < total; ++i)
        {
            if (ui->triggersList->item(i, 0)->text().compare(object, Qt::CaseInsensitive) == 0)
            {
                ui->triggersList->removeRow(i);
                return;
            }
        }
    }

    if (type != DbObjectType::VIEW)
        return;

    if (modifyingThisView)
        return;

    // TODO uncomment below when dbnames are supported
//    if (this->database != database)
//        return;

    if (object.compare(view, Qt::CaseInsensitive) == 0)
    {
        dbClosedFinalCleanup();
        MDIAREA->enforceCurrentTaskSelectionAfterWindowClose();
        getMdiWindow()->close();
    }
}

void ViewWindow::updateOutputColumnsVisibility()
{
    bool enabled = outputColumnsCheck->isChecked();

    ui->outputColumnsContainer->setVisible(enabled);
    actionMap[Action::GENERATE_OUTPUT_COLUMNS]->setVisible(enabled);
    actionMap[Action::ADD_COLUMN]->setVisible(enabled);
    actionMap[Action::EDIT_COLUMN]->setVisible(enabled);
    actionMap[Action::DEL_COLUMN]->setVisible(enabled);
    actionMap[Action::MOVE_COLUMN_UP]->setVisible(enabled);
    actionMap[Action::MOVE_COLUMN_DOWN]->setVisible(enabled);

    updateQueryToolbarStatus();
}

void ViewWindow::addColumn()
{
    QListWidgetItem* item = new QListWidgetItem();
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    ui->outputColumnsTable->addItem(item);
    ui->outputColumnsTable->editItem(item);
    ui->outputColumnsTable->setCurrentItem(item);
    updateColumnButtons();
}

void ViewWindow::editColumn()
{
    QListWidgetItem* item = ui->outputColumnsTable->currentItem();
    ui->outputColumnsTable->editItem(item);
    updateColumnButtons();
}

void ViewWindow::delColumn()
{
    QListWidgetItem* item = ui->outputColumnsTable->takeItem(ui->outputColumnsTable->currentRow());
    delete item;
    updateColumnButtons();
}

void ViewWindow::moveColumnUp()
{
    int row = ui->outputColumnsTable->currentRow();
    if (row <= 0)
        return;

    QListWidgetItem* item = ui->outputColumnsTable->takeItem(row);
    ui->outputColumnsTable->insertItem(--row, item);
    ui->outputColumnsTable->setCurrentItem(item);
}

void ViewWindow::moveColumnDown()
{
    int row = ui->outputColumnsTable->currentRow();
    if (row + 1 >= ui->outputColumnsTable->count())
        return;

    QListWidgetItem* item = ui->outputColumnsTable->takeItem(row);
    ui->outputColumnsTable->insertItem(++row, item);
    ui->outputColumnsTable->setCurrentItem(item);
}

void ViewWindow::updateColumnButtons()
{
    QListWidgetItem* item = ui->outputColumnsTable->currentItem();
    int row = ui->outputColumnsTable->currentRow();

    actionMap[MOVE_COLUMN_UP]->setEnabled(row > 0);
    actionMap[MOVE_COLUMN_DOWN]->setEnabled(row + 1 < ui->outputColumnsTable->count());
    actionMap[EDIT_COLUMN]->setEnabled(item != nullptr);
    actionMap[DEL_COLUMN]->setEnabled(item != nullptr);
}

void ViewWindow::generateOutputColumns()
{
    if (ui->outputColumnsTable->count() > 0)
    {
        QMessageBox::StandardButton res = QMessageBox::question(this, tr("Override columns"), tr("Currently defined columns will be overriden. Do you want to continue?"));
        if (res != QMessageBox::Yes)
            return;
    }

    // Validate and generate fresh createView instance
    bool validated = validate(true);
    if (!validated)
        return;

    // Make copy of CREATE statement and remove columns
    SqliteCreateView* stmt = dynamic_cast<SqliteCreateView*>(createView->clone());
    for (SqliteIndexedColumn* col : stmt->columns)
        delete col;

    stmt->columns.clear();

    // Indentify columns
    SchemaResolver resolver(db);
    QStringList columns = resolver.getColumnsUsingPragma(stmt);
    delete stmt;
    if (columns.isEmpty())
    {
        notifyWarn(tr("Could not determinate columns returned from the view. The query is problably incomplete or contains errors."));
        return;
    }

    ui->outputColumnsTable->clear();
    ui->outputColumnsTable->addItems(columns);

    QListWidgetItem* item = nullptr;
    for (int row = 0; row < columns.size(); row++)
    {
        item = ui->outputColumnsTable->item(row);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
    }
}

void ViewWindow::updateTabsOrder()
{
    tabsMoving = true;
    ui->tabWidget->removeTab(getDataTabIdx());
    int idx = 1;
    if (CFG_UI.General.DataTabAsFirstInViews.get())
        idx = 0;

    ui->tabWidget->insertTab(idx, ui->dataTab, tr("Data"));
    tabsMoving = false;
}

void ViewWindow::triggerViewDoubleClicked(const QModelIndex& idx)
{
    if (!idx.isValid())
    {
        addTrigger();
        return;
    }

    QString trigger = ui->triggersList->item(idx.row(), 0)->text();

    DbObjectDialogs dialogs(db, this);
    dialogs.editTrigger(trigger);
    refreshTriggers();
}

void ViewWindow::refreshTriggers()
{
    if (!db || !db->isValid())
        return;

    SchemaResolver resolver(db);
    QList<SqliteCreateTriggerPtr> triggers = resolver.getParsedTriggersForView(database, view);

    ui->triggersList->setColumnCount(4);
    ui->triggersList->setRowCount(triggers.size());
    ui->triggersList->horizontalHeader()->setMaximumSectionSize(200);
    ui->triggersList->setHorizontalHeaderLabels({
                                                 tr("Name", "view window triggers"),
                                                 tr("Instead of", "view window triggers"),
                                                 tr("Condition", "view window triggers"),
                                                 tr("Details", "table window triggers")
                                             });

    QTableWidgetItem* item = nullptr;
    QString event;
    int row = 0;
    for (SqliteCreateTriggerPtr trig : triggers)
    {
        item = new QTableWidgetItem(trig->trigger);
        item->setFlags(Qt::ItemIsEnabled|Qt::ItemIsSelectable);
        ui->triggersList->setItem(row, 0, item);

        event = trig->tokensMap["trigger_event"].detokenize();
        item = new QTableWidgetItem(event);
        item->setFlags(Qt::ItemIsEnabled|Qt::ItemIsSelectable);
        ui->triggersList->setItem(row, 1, item);

        item = new QTableWidgetItem(trig->precondition ? trig->precondition->detokenize().trimmed() : "");
        item->setFlags(Qt::ItemIsEnabled|Qt::ItemIsSelectable);
        ui->triggersList->setItem(row, 2, item);

        item = new QTableWidgetItem(trig->tokensMap["trigger_cmd_list"].detokenize().trimmed());
        item->setFlags(Qt::ItemIsEnabled|Qt::ItemIsSelectable);
        ui->triggersList->setItem(row, 3, item);

        row++;
    }

    ui->triggersList->resizeColumnsToContents();
    updateTriggersState();
}

void ViewWindow::parseDdl()
{
    if (existingView)
    {
        SchemaResolver resolver(db);
        SqliteQueryPtr parsedObject = resolver.getParsedObject(database, view, SchemaResolver::VIEW);
        if (!parsedObject.dynamicCast<SqliteCreateView>())
        {
            notifyError(tr("Could not process the %1 view correctly. Unable to open a view window.").arg(view));
            invalid = true;
            return;
        }

        createView = parsedObject.dynamicCast<SqliteCreateView>();
    }
    else
    {
        createView = SqliteCreateViewPtr::create();
        createView->view = view;
    }
    originalCreateView = SqliteCreateViewPtr::create(*createView);

    // Replacing \r\n with \n, cause \r\n can be carried over from version 2.x.x, which did this incorrectly.
    originalQuery = originalCreateView->select->detokenize().replace("\r\n", "\n");
}

void ViewWindow::updateDdlTab()
{
    ui->ddlEdit->setPlainText(FORMATTER->format("sql", getCurrentDdl(), db));
}

bool ViewWindow::isModified() const
{
    // Quick checks first
    bool modified = !existingView || (originalCreateView && originalCreateView->view != ui->nameEdit->text()) ||
            ui->queryEdit->toPlainText() != originalQuery;

    if (modified)
        return modified;

    // And now a bit slower check
    QStringList origCols = createView ? indexedColumnsToNamesOnly(createView->columns) : QStringList();
    QStringList currentCols;
    if (outputColumnsCheck->isChecked())
        currentCols = collectColumnNames();

    bool colsModified = origCols != currentCols;
    return colsModified;
}

bool ViewWindow::validate(bool skipWarnings)
{
    if (!existingView && !skipWarnings && ui->nameEdit->text().isEmpty())
    {
        int res = QMessageBox::warning(this, tr("Empty name"), tr("A blank name for the view is allowed in SQLite, but it is not recommended.\n"
            "Are you sure you want to create a view with blank name?"), QMessageBox::Yes, QMessageBox::No);

        if (res != QMessageBox::Yes)
            return false;
    }

    // Rebuilding createView statement and validating it on the fly.
    QString ddl = getCurrentDdl();
    Parser parser;
    if (!parser.parse(ddl) || parser.getQueries().size() < 1)
    {
        notifyError(tr("The SELECT statement could not be parsed. Please correct the query and retry.\nDetails: %1").arg(parser.getErrorString()));
        return false;
    }

    SqliteQueryPtr query = parser.getQueries().first();
    SqliteCreateViewPtr viewStmt = query.dynamicCast<SqliteCreateView>();
    if (!viewStmt)
    {
        notifyError(tr("The view could not be modified due to internal SQLiteStudio error. Please report this!"));
        qCritical() << "Could not parse new view, because parsed object is of different type. The type is"
                    << sqliteQueryTypeToString(query->queryType) << "for following query:" << ddl;
        return false;
    }

    createView = viewStmt;
    return true;
}

void ViewWindow::executeStructureChanges()
{
    QStringList sqls;
    QList<bool> sqlMandatoryFlags;

    QString theDdl = getCurrentDdl();
    if (!existingView)
    {
        sqls << theDdl;
    }
    else
    {
        Parser parser;
        if (!parser.parse(theDdl))
        {
            qCritical() << "Could not re-parse the view for executing it:" << parser.getErrorString();
            notifyError(tr("The view code could not be parsed properly for execution. This is a SQLiteStudio's bug. Please report it."));
            return;
        }

        createView = parser.getQueries().first().dynamicCast<SqliteCreateView>();
        if (viewModifier)
            delete viewModifier;

        viewModifier = new ViewModifier(db, database, view);
        viewModifier->alterView(createView);

        if (viewModifier->hasMessages())
        {
            MessageListDialog dialog(tr("Following problems will take place while modifying the view.\n"
                                        "Would you like to proceed?", "view window"));
            dialog.setWindowTitle(tr("View modification", "view window"));
            for (const QString& error : viewModifier->getErrors())
                dialog.addError(error);

            for (const QString& warn : viewModifier->getWarnings())
                dialog.addWarning(warn);

            if (dialog.exec() != QDialog::Accepted)
                return;
        }

        sqls = viewModifier->generateSqls();
        sqlMandatoryFlags = viewModifier->getMandatoryFlags();
    }

    if (!CFG_UI.General.DontShowDdlPreview.get())
    {
        DdlPreviewDialog dialog(db, this);
        dialog.setDdl(sqls);
        if (dialog.exec() != QDialog::Accepted)
            return;
    }

    modifyingThisView = true;
    structureExecutor->setDb(db);
    structureExecutor->setQueries(sqls);
    structureExecutor->setMandatoryQueries(sqlMandatoryFlags);
    structureExecutor->exec();
    widgetCover->show();
}

void ViewWindow::updateFont()
{
    QFont f = CFG_UI.Fonts.DataView.get();
    QFontMetrics fm(f);

    QTableView* views[] = {ui->triggersList};
    for (QTableView* view : views)
    {
        view->setFont(f);
        view->horizontalHeader()->setFont(f);
        view->verticalHeader()->setFont(f);
        view->verticalHeader()->setDefaultSectionSize(fm.height() + 4);
    }
}

void ViewWindow::dbChanged()
{
    if (db)
        disconnect(db, SIGNAL(dbObjectDeleted(QString,QString,DbObjectType)), this, SLOT(checkIfViewDeleted(QString,QString,DbObjectType)));

    db = ui->dbCombo->currentDb();
    dataModel->setDb(db);
    ui->queryEdit->setDb(db);

    if (db)
        connect(db, SIGNAL(dbObjectDeleted(QString,QString,DbObjectType)), this, SLOT(checkIfViewDeleted(QString,QString,DbObjectType)));
}

void ViewWindow::handleObjectModified(Db* db, const QString& database, const QString& object)
{
    UNUSED(db);
    UNUSED(database);
    if (object.compare(view, Qt::CaseInsensitive) != 0)
        return;

// TODO uncomment below when dbnames are supported
//    if (this->database != database)
//        return;

    view = object;
    refreshView();
}
