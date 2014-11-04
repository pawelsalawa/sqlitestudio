#include "viewwindow.h"
#include "ui_viewwindow.h"
#include "common/unused.h"
#include "schemaresolver.h"
#include "services/notifymanager.h"
#include "services/dbmanager.h"
#include "mainwindow.h"
#include "mdiarea.h"
#include "sqlitesyntaxhighlighter.h"
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
#include <QPushButton>
#include <QProgressBar>
#include <QDebug>
#include <QMessageBox>

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
        notifyWarn("Could not restore window, because no database or view was stored in session for this window.");
        return false;
    }

    if (!value.contains("db") || !value.contains("view"))
    {
        notifyWarn("Could not restore window, because no database or view was stored in session for this window.");
        return false;
    }

    db = DBLIST->getByName(value["db"].toString());
    if (!db)
    {
        notifyWarn(tr("Could not restore window, because database %1 could not be resolved.").arg(value["db"].toString()));
        return false;
    }

    if (!db->isOpen() && !db->open())
    {
        notifyWarn(tr("Could not restore window, because database %1 could not be open.").arg(value["db"].toString()));
        return false;
    }

    view = value["view"].toString();
    database = value["database"].toString();
    SchemaResolver resolver(db);
    if (!resolver.getViews(database).contains(view, Qt::CaseInsensitive))
    {
        notifyWarn(tr("Could not restore window, because the view %1 doesn't exist in the database %2.").arg(view).arg(db->getName()));
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
                           REFRESH_TRIGGERS,
                           ADD_TRIGGER,
                           EDIT_TRIGGER,
                           DEL_TRIGGER,
                       },
                       Qt::WidgetWithChildrenShortcut);

    BIND_SHORTCUTS(ViewWindow, Action);
}

bool ViewWindow::restoreSessionNextTime()
{
    return existingView;
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

    dataModel = new SqlQueryModel(this);
    ui->dataView->init(dataModel);

    ui->queryEdit->setVirtualSqlExpression("CREATE VIEW name AS %1");
    ui->queryEdit->setDb(db);

    connect(dataModel, SIGNAL(executionSuccessful()), this, SLOT(executionSuccessful()));
    connect(dataModel, SIGNAL(executionFailed(QString)), this, SLOT(executionFailed(QString)));
    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));
    connect(ui->nameEdit, SIGNAL(textChanged(QString)), this, SLOT(updateQueryToolbarStatus()));
    connect(ui->queryEdit, SIGNAL(textChanged()), this, SLOT(updateQueryToolbarStatus()));
    connect(ui->queryEdit, SIGNAL(errorsChecked(bool)), this, SLOT(updateQueryToolbarStatus()));
    connect(ui->triggersList, SIGNAL(itemSelectionChanged()), this, SLOT(updateTriggersState()));

    structureExecutor = new ChainExecutor(this);
    connect(structureExecutor, SIGNAL(success()), this, SLOT(changesSuccessfullyCommited()));
    connect(structureExecutor, SIGNAL(failure(int,QString)), this, SLOT(changesFailedToCommit(int,QString)));

    setupCoverWidget();

    initActions();

    refreshTriggers();
    updateQueryToolbarStatus();
    updateTriggersState();
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

    if (existingView)
    {
        dataModel->setDb(db);
        dataModel->setQuery(originalCreateView->select->detokenize());
    }

    ui->queryEdit->setDb(db);
    ui->queryEdit->setPlainText(createView->select->detokenize());
    updateDdlTab();

    ui->ddlEdit->setSqliteVersion(db->getVersion());

    refreshTriggers();

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
    createAction(REFRESH_QUERY, ICONS.RELOAD, tr("Refresh the view", "view window"), this, SLOT(refreshView()), ui->queryToolbar);
    ui->queryToolbar->addSeparator();
    createAction(COMMIT_QUERY, ICONS.COMMIT, tr("Commit the view changes", "view window"), this, SLOT(commitView()), ui->queryToolbar);
    createAction(ROLLBACK_QUERY, ICONS.ROLLBACK, tr("Rollback the view changes", "view window"), this, SLOT(rollbackView()), ui->queryToolbar);
    ui->queryToolbar->addSeparator();
    ui->queryToolbar->addAction(ui->queryEdit->getAction(SqlEditor::FORMAT_SQL));
}

void ViewWindow::createTriggersTabActions()
{
    createAction(REFRESH_TRIGGERS, ICONS.RELOAD, tr("Refresh trigger list", "view window"), this, SLOT(refreshTriggers()), ui->triggersToolbar, ui->triggersList);
    ui->triggersToolbar->addSeparator();
    createAction(ADD_TRIGGER, ICONS.TRIGGER_ADD, tr("Create new triger", "view window"), this, SLOT(addTrigger()), ui->triggersToolbar, ui->triggersList);
    createAction(EDIT_TRIGGER, ICONS.TRIGGER_EDIT, tr("Edit selected triger", "view window"), this, SLOT(editTrigger()), ui->triggersToolbar, ui->triggersList);
    createAction(DEL_TRIGGER, ICONS.TRIGGER_DEL, tr("Delete selected triger", "view window"), this, SLOT(deleteTrigger()), ui->triggersToolbar, ui->triggersList);
}
QString ViewWindow::getView() const
{
    return view;
}

void ViewWindow::setSelect(const QString &selectSql)
{
    ui->queryEdit->setPlainText(selectSql);
}

bool ViewWindow::isUncommited() const
{
    return ui->dataView->isUncommited() || isModified();
}

QString ViewWindow::getQuitUncommitedConfirmMessage() const
{
    QString title = getMdiWindow()->windowTitle();
    if (ui->dataView->isUncommited() && isModified())
        return tr("View window \"%1\" has uncommited structure modifications and data.").arg(title);
    else if (ui->dataView->isUncommited())
        return tr("View window \"%1\" has uncommited data.").arg(title);
    else if (isModified())
        return tr("View window \"%1\" has uncommited structure modifications.").arg(title);
    else
    {
        qCritical() << "Unhandled message case in ViewWindow::getQuitUncommitedConfirmMessage().";
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

void ViewWindow::commitView(bool skipWarnings)
{
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

    updateQueryToolbarStatus();
    updateDdlTab();
}

QString ViewWindow::getCurrentTrigger() const
{
    int row = ui->triggersList->currentRow();
    QTableWidgetItem* item = ui->triggersList->item(row, 0);
    if (!item)
        return QString::null;

    return item->text();
}

void ViewWindow::applyInitialTab()
{
    if (existingView && !view.isNull() && CFG_UI.General.OpenViewsOnData.get())
        ui->tabWidget->setCurrentIndex(1);
    else
        ui->tabWidget->setCurrentIndex(0);
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
    dialogs.dropObject(trigger);
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
    notifyError(tr("Could not load data for view %1. Error details: %2").arg(view).arg(errorMessage));
}

void ViewWindow::tabChanged(int tabIdx)
{
    switch (tabIdx)
    {
        case 1:
        {
            if (isModified())
            {
                int res = QMessageBox::question(this, tr("Uncommited changes"),
                                                tr("There are uncommited structure modifications. You cannot browse or edit data until you have "
                                                   "the view structure settled.\n"
                                                   "Do you want to commit the structure, or do you want to go back to the structure tab?"),
                                                tr("Go back to structure tab"), tr("Commit modifications and browse data."));

                ui->tabWidget->setCurrentIndex(0);
                if (res == 1)
                    commitView(true);

                break;
            }

            if (!dataLoaded)
                ui->dataView->refreshData();

            break;
        }
        case 3:
        {
            updateDdlTab();
            break;
        }
    }
}

void ViewWindow::updateQueryToolbarStatus()
{
    bool modified = isModified();
    bool queryOk = ui->queryEdit->isSyntaxChecked() && !ui->queryEdit->haveErrors();
    actionMap[COMMIT_QUERY]->setEnabled(modified && queryOk);
    actionMap[ROLLBACK_QUERY]->setEnabled(modified && existingView);
    actionMap[REFRESH_QUERY]->setEnabled(existingView);
}

void ViewWindow::changesSuccessfullyCommited()
{
    QStringList sqls = structureExecutor->getQueries();
    CFG->addDdlHistory(sqls.join("\n"), db->getName(), db->getPath());

    widgetCover->hide();

    originalCreateView = createView;
    dataLoaded = false;

    //QString oldView = view; // uncomment when implementing notify manager call
    database = createView->database;
    view = createView->view;
    existingView = true;
    initView();
    updateQueryToolbarStatus();
    updateWindowTitle();

    DBTREE->refreshSchema(db);
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
    dataModel->setDb(nullptr);
    ui->queryEdit->setDb(nullptr);
    structureExecutor->setDb(nullptr);
}

void ViewWindow::checkIfViewDeleted(const QString& database, const QString& object, DbObjectType type)
{
    UNUSED(database);
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
        getMdiWindow()->close();
    }
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

    QTableWidgetItem* item;
    QString event;
    int row = 0;
    foreach (SqliteCreateTriggerPtr trig, triggers)
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
        createView->dialect = db->getDialect();
    }
    originalCreateView = SqliteCreateViewPtr::create(*createView);
    originalQuery = originalCreateView->select->detokenize();
}

void ViewWindow::updateDdlTab()
{
    QString ddl = "CREATE VIEW %1 AS %2";
    ui->ddlEdit->setPlainText(ddl.arg(wrapObjIfNeeded(ui->nameEdit->text(), db->getDialect())).arg(ui->queryEdit->toPlainText()));
}

bool ViewWindow::isModified() const
{
    return (originalCreateView && originalCreateView->view != ui->nameEdit->text()) ||
            ui->queryEdit->toPlainText() != originalQuery ||
            !existingView;
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
    QString ddl = "CREATE VIEW %1 AS %2";
    QString viewName = wrapObjIfNeeded(ui->nameEdit->text(), db->getDialect());
    QString select = ui->queryEdit->toPlainText();

    Parser parser(db->getDialect());
    if (!parser.parse(ddl.arg(viewName).arg(select)) || parser.getQueries().size() < 1)
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

    createView->rebuildTokens();
    if (!existingView)
    {
        sqls << createView->detokenize();
    }
    else
    {
        if (viewModifier)
            delete viewModifier;

        viewModifier = new ViewModifier(db, database, view);
        viewModifier->alterView(createView);

        if (viewModifier->hasMessages())
        {
            MessageListDialog dialog(tr("Following problems will take place while modifying the view.\n"
                                        "Would you like to proceed?", "view window"));
            dialog.setWindowTitle(tr("View modification", "view window"));
            foreach (const QString& error, viewModifier->getErrors())
                dialog.addError(error);

            foreach (const QString& warn, viewModifier->getWarnings())
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
