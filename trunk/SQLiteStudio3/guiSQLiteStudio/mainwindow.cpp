#include "mainwindow.h"
#include "common/unused.h"
#include "dbtree/dbtree.h"
#include "dbtree/dbtreemodel.h"
#include "iconmanager.h"
#include "windows/editorwindow.h"
#include "windows/tablewindow.h"
#include "windows/viewwindow.h"
#include "windows/functionseditor.h"
#include "windows/collationseditor.h"
#include "windows/ddlhistorywindow.h"
#include "mdiarea.h"
#include "statusfield.h"
#include "uiconfig.h"
#include "common/extaction.h"
#include "dbobjectdialogs.h"
#include "services/notifymanager.h"
#include "dialogs/configdialog.h"
#include "services/pluginmanager.h"
#include "formmanager.h"
#include "customconfigwidgetplugin.h"
#include "sqlitesyntaxhighlighter.h"
#include "qtscriptsyntaxhighlighter.h"
#include "services/exportmanager.h"
#include "services/importmanager.h"
#include "dialogs/exportdialog.h"
#include "dialogs/importdialog.h"
#include "multieditor/multieditorwidgetplugin.h"
#include "multieditor/multieditor.h"
#include "dialogs/dbdialog.h"
#include "uidebug.h"
#include "services/dbmanager.h"
#include "services/updatemanager.h"
#include "dialogs/aboutdialog.h"
#include "dialogs/bugdialog.h"
#include "windows/bugreporthistorywindow.h"
#include "dialogs/newversiondialog.h"
#include "dialogs/quitconfirmdialog.h"
#include "common/widgetcover.h"
#include <QMdiSubWindow>
#include <QDebug>
#include <QStyleFactory>
#include <QUiLoader>
#include <QInputDialog>
#include <QProgressBar>
#include <QLabel>

CFG_KEYS_DEFINE(MainWindow)
MainWindow* MainWindow::instance = nullptr;

MainWindow::MainWindow() :
    QMainWindow(),
    ui(new Ui::MainWindow)
{
    init();
}

MainWindow::~MainWindow()
{
}

void MainWindow::init()
{
    ui->setupUi(this);
    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(cleanUp()));

#ifdef Q_OS_WIN
    setWindowIcon(ICONS.SQLITESTUDIO_APP.toQIcon().pixmap(256, 256));
#else
    setWindowIcon(ICONS.SQLITESTUDIO_APP);
#endif

    setWindowTitle(QString("SQLiteStudio (%1)").arg(SQLITESTUDIO->getVersionString()));

#ifdef Q_OS_MACX
    ui->centralWidget->layout()->setMargin(0);
#endif

    Committable::init(MainWindow::confirmQuit);

    DbTreeModel::staticInit();
    dbTree = new DbTree(this);
    addDockWidget(Qt::LeftDockWidgetArea, dbTree);

    statusField = new StatusField(this);
    addDockWidget(Qt::BottomDockWidgetArea, statusField);
    if (!statusField->hasMessages())
        statusField->close();

    initActions();

    ui->mdiArea->setTaskBar(ui->taskBar);
    addToolBar(Qt::BottomToolBarArea, ui->taskBar);

    addToolBar(Qt::TopToolBarArea, ui->viewToolbar);
    insertToolBar(ui->viewToolbar, ui->mainToolBar);
    insertToolBar(ui->mainToolBar, ui->structureToolbar);
    insertToolBar(ui->structureToolbar, ui->dbToolbar);

    formManager = new FormManager();

    initMenuBar();

    PLUGINS->registerPluginType<CustomConfigWidgetPlugin>(tr("Configuration widgets"));
    PLUGINS->registerPluginType<SyntaxHighlighterPlugin>(tr("Syntax highlighting engines"));
    PLUGINS->registerPluginType<MultiEditorWidgetPlugin>(tr("Data editors"));
    PLUGINS->loadBuiltInPlugin(new SqliteHighlighterPlugin);
    PLUGINS->loadBuiltInPlugin(new JavaScriptHighlighterPlugin);
    MultiEditor::loadBuiltInEditors();

    updateWindowActions();

    qApp->installEventFilter(this);

    if (isDebugEnabled())
    {
        if (isDebugConsoleEnabled())
            notifyInfo(tr("Running in debug mode. Press %1 or use 'Help / Open debug console' menu entry to open the debug console.").arg(shortcuts[OPEN_DEBUG_CONSOLE]->get()));
        else
            notifyInfo(tr("Running in debug mode. Debug messages are printed to the standard output."));
    }

    connect(UPDATES, SIGNAL(updatesAvailable(QList<UpdateManager::UpdateEntry>)), this, SLOT(updatesAvailable(QList<UpdateManager::UpdateEntry>)));
    connect(UPDATES, SIGNAL(noUpdatesAvailable()), this, SLOT(noUpdatesAvailable()));
    connect(statusField, SIGNAL(linkActivated(QString)), this, SLOT(statusFieldLinkClicked(QString)));

    // Widget cover
    widgetCover = new WidgetCover(this);
    widgetCover->setVisible(false);

    updatingBusyBar = new QProgressBar();
    updatingBusyBar->setRange(0, 100);
    updatingBusyBar->setTextVisible(true);
    updatingBusyBar->setValue(0);
    updatingBusyBar->setFixedWidth(300);

    updatingSubBar = new QProgressBar();
    updatingSubBar->setRange(0, 100);
    updatingSubBar->setTextVisible(true);
    updatingSubBar->setValue(0);
    updatingSubBar->setFixedWidth(300);

    updatingLabel = new QLabel();

    widgetCover->getContainerLayout()->addWidget(updatingLabel, 0, 0);
    widgetCover->getContainerLayout()->addWidget(updatingBusyBar, 1, 0);
    widgetCover->getContainerLayout()->addWidget(updatingSubBar, 2, 0);
    connect(UPDATES, SIGNAL(updatingProgress(QString,int,int)), this, SLOT(handleUpdatingProgress(QString,int,int)));
    connect(UPDATES, SIGNAL(updatingError(QString)), this, SLOT(handleUpdatingError()));
}

void MainWindow::cleanUp()
{
    if (SQLITESTUDIO->getImmediateQuit())
        return;

    for (MdiWindow* win : getMdiArea()->getWindows())
        delete win;

    removeDockWidget(dbTree);
    removeDockWidget(statusField);

    safe_delete(dbTree);
    safe_delete(statusField);

    delete ui;

    safe_delete(formManager);
}

EditorWindow* MainWindow::openSqlEditor()
{
    EditorWindow* win = new EditorWindow(ui->mdiArea);
    if (win->isInvalid())
    {
        delete win;
        return nullptr;
    }

    ui->mdiArea->addSubWindow(win);
    return win;
}

void MainWindow::updateWindowActions()
{
    bool hasActiveTask = ui->mdiArea->activeSubWindow();
    actionMap[MDI_CASCADE]->setEnabled(hasActiveTask);
    actionMap[MDI_TILE]->setEnabled(hasActiveTask);
    actionMap[MDI_TILE_HORIZONTAL]->setEnabled(hasActiveTask);
    actionMap[MDI_TILE_VERTICAL]->setEnabled(hasActiveTask);
    actionMap[CLOSE_WINDOW]->setEnabled(hasActiveTask);
    actionMap[CLOSE_OTHER_WINDOWS]->setEnabled(hasActiveTask);
    actionMap[CLOSE_ALL_WINDOWS]->setEnabled(hasActiveTask);
    actionMap[RENAME_WINDOW]->setEnabled(hasActiveTask);
    actionMap[RESTORE_WINDOW]->setEnabled(hasClosedWindowToRestore());
}

MdiArea *MainWindow::getMdiArea() const
{
    return dynamic_cast<MdiArea*>(ui->mdiArea);
}

DbTree *MainWindow::getDbTree() const
{
    return dbTree;
}

StatusField *MainWindow::getStatusField() const
{
    return statusField;
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (SQLITESTUDIO->getImmediateQuit())
    {
        closingApp = true;
        QMainWindow::closeEvent(event);
        return;
    }

    if (!Committable::canQuit())
    {
        event->ignore();
        return;
    }

    closingApp = true;
    closeNonSessionWindows();
    MdiWindow* currWindow = ui->mdiArea->getCurrentWindow();
    hide();
    saveSession(currWindow);
    QMainWindow::closeEvent(event);
}

void MainWindow::createActions()
{
    createAction(OPEN_SQL_EDITOR, ICONS.OPEN_SQL_EDITOR, tr("Open SQL editor"), this, SLOT(openSqlEditorSlot()), ui->mainToolBar);
    createAction(OPEN_DDL_HISTORY, ICONS.DDL_HISTORY, tr("Open DDL history"), this, SLOT(openDdlHistorySlot()), ui->mainToolBar);
    createAction(OPEN_FUNCTION_EDITOR, ICONS.FUNCTION, tr("Open SQL functions editor"), this, SLOT(openFunctionEditorSlot()), ui->mainToolBar);
    createAction(OPEN_COLLATION_EDITOR, ICONS.CONSTRAINT_COLLATION, tr("Open collations editor"), this, SLOT(openCollationEditorSlot()), ui->mainToolBar);
    createAction(IMPORT, ICONS.IMPORT, tr("Import"), this, SLOT(importAnything()), ui->mainToolBar);
    createAction(EXPORT, ICONS.EXPORT, tr("Export"), this, SLOT(exportAnything()), ui->mainToolBar);
    ui->mainToolBar->addSeparator();
    createAction(OPEN_CONFIG, ICONS.CONFIGURE, tr("Open configuration dialog"), this, SLOT(openConfig()), ui->mainToolBar);

    createAction(MDI_TILE, ICONS.WIN_TILE, tr("Tile windows"), ui->mdiArea, SLOT(tileSubWindows()), ui->viewToolbar);
    createAction(MDI_TILE_HORIZONTAL, ICONS.WIN_TILE_HORIZONTAL, tr("Tile windows horizontally"), ui->mdiArea, SLOT(tileHorizontally()), ui->viewToolbar);
    createAction(MDI_TILE_VERTICAL, ICONS.WIN_TILE_VERTICAL, tr("Tile windows vertically"), ui->mdiArea, SLOT(tileVertically()), ui->viewToolbar);
    createAction(MDI_CASCADE, ICONS.WIN_CASCADE, tr("Cascade windows"), ui->mdiArea, SLOT(cascadeSubWindows()), ui->viewToolbar);
    createAction(NEXT_TASK, tr("Next window"), ui->taskBar, SLOT(nextTask()), this);
    createAction(PREV_TASK, tr("Previous window"), ui->taskBar, SLOT(prevTask()), this);
    createAction(HIDE_STATUS_FIELD, tr("Hide status field"), this, SLOT(hideStatusField()), this);

    createAction(CLOSE_WINDOW, ICONS.WIN_CLOSE, tr("Close selected window"), this, SLOT(closeSelectedWindow()), this);
    createAction(CLOSE_OTHER_WINDOWS, ICONS.WIN_CLOSE_OTHER, tr("Close all windows but selected"), this, SLOT(closeAllWindowsButSelected()), this);
    createAction(CLOSE_ALL_WINDOWS, ICONS.WIN_CLOSE_ALL, tr("Close all windows"), this, SLOT(closeAllWindows()), this);
    createAction(RESTORE_WINDOW, ICONS.WIN_RESTORE, tr("Restore recently closed window"), this, SLOT(restoreLastClosedWindow()), this);
    createAction(RENAME_WINDOW, ICONS.WIN_RENAME, tr("Rename selected window"), this, SLOT(renameWindow()), this);

    createAction(OPEN_DEBUG_CONSOLE, tr("Open Debug Console"), this, SLOT(openDebugConsole()), this);
    createAction(REPORT_BUG, ICONS.BUG, tr("Report a bug"), this, SLOT(reportBug()), this);
    createAction(FEATURE_REQUEST, ICONS.FEATURE_REQUEST, tr("Propose a new feature"), this, SLOT(requestFeature()), this);
    createAction(ABOUT, ICONS.SQLITESTUDIO_APP16, tr("About"), this, SLOT(aboutSqlitestudio()), this);
    createAction(LICENSES, ICONS.LICENSES, tr("Licenses"), this, SLOT(licenses()), this);
    createAction(HOMEPAGE, ICONS.HOMEPAGE, tr("Open home page"), this, SLOT(homepage()), this);
    createAction(FORUM, ICONS.OPEN_FORUM, tr("Open forum page"), this, SLOT(forum()), this);
    createAction(USER_MANUAL, ICONS.USER_MANUAL, tr("User Manual"), this, SLOT(userManual()), this);
    createAction(SQLITE_DOCS, ICONS.SQLITE_DOCS, tr("SQLite documentation"), this, SLOT(sqliteDocs()), this);
    createAction(BUG_REPORT_HISTORY, ICONS.BUG_LIST, tr("Report history"), this, SLOT(reportHistory()), this);
    createAction(CHECK_FOR_UPDATES, ICONS.GET_UPDATE, tr("Check for updates"), this, SLOT(checkForUpdates()), this);

    actionMap[ABOUT]->setMenuRole(QAction::AboutRole);
    actionMap[OPEN_CONFIG]->setMenuRole(QAction::PreferencesRole);

    ui->dbToolbar->addAction(dbTree->getAction(DbTree::CONNECT_TO_DB));
    ui->dbToolbar->addAction(dbTree->getAction(DbTree::DISCONNECT_FROM_DB));
    ui->dbToolbar->addSeparator();
    ui->dbToolbar->addAction(dbTree->getAction(DbTree::ADD_DB));
    ui->dbToolbar->addAction(dbTree->getAction(DbTree::EDIT_DB));
    ui->dbToolbar->addAction(dbTree->getAction(DbTree::DELETE_DB));
    ui->dbToolbar->addSeparator();
    ui->dbToolbar->addAction(dbTree->getAction(DbTree::REFRESH_SCHEMA));

    ui->structureToolbar->addAction(dbTree->getAction(DbTree::ADD_TABLE));
    ui->structureToolbar->addAction(dbTree->getAction(DbTree::EDIT_TABLE));
    ui->structureToolbar->addAction(dbTree->getAction(DbTree::DEL_TABLE));
    ui->structureToolbar->addSeparator();
    ui->structureToolbar->addAction(dbTree->getAction(DbTree::ADD_INDEX));
    ui->structureToolbar->addAction(dbTree->getAction(DbTree::EDIT_INDEX));
    ui->structureToolbar->addAction(dbTree->getAction(DbTree::DEL_INDEX));
    ui->structureToolbar->addSeparator();
    ui->structureToolbar->addAction(dbTree->getAction(DbTree::ADD_TRIGGER));
    ui->structureToolbar->addAction(dbTree->getAction(DbTree::EDIT_TRIGGER));
    ui->structureToolbar->addAction(dbTree->getAction(DbTree::DEL_TRIGGER));
    ui->structureToolbar->addSeparator();
    ui->structureToolbar->addAction(dbTree->getAction(DbTree::ADD_VIEW));
    ui->structureToolbar->addAction(dbTree->getAction(DbTree::EDIT_VIEW));
    ui->structureToolbar->addAction(dbTree->getAction(DbTree::DEL_VIEW));

    ui->taskBar->initContextMenu(this);
}

void MainWindow::initMenuBar()
{
    // Database menu
    dbMenu = new QMenu(this);
    dbMenu->setTitle(tr("Database", "menubar"));
    menuBar()->addMenu(dbMenu);

    dbMenu->addAction(dbTree->getAction(DbTree::CONNECT_TO_DB));
    dbMenu->addAction(dbTree->getAction(DbTree::DISCONNECT_FROM_DB));
    dbMenu->addSeparator();
    dbMenu->addAction(dbTree->getAction(DbTree::ADD_DB));
    dbMenu->addAction(dbTree->getAction(DbTree::EDIT_DB));
    dbMenu->addAction(dbTree->getAction(DbTree::DELETE_DB));
    dbMenu->addSeparator();
    dbMenu->addAction(dbTree->getAction(DbTree::EXPORT_DB));
    dbMenu->addAction(dbTree->getAction(DbTree::CONVERT_DB));
    dbMenu->addAction(dbTree->getAction(DbTree::VACUUM_DB));
    dbMenu->addAction(dbTree->getAction(DbTree::INTEGRITY_CHECK));
    dbMenu->addSeparator();
    dbMenu->addAction(dbTree->getAction(DbTree::REFRESH_SCHEMA));
    dbMenu->addAction(dbTree->getAction(DbTree::REFRESH_SCHEMAS));

    // Structure menu
    structMenu = new QMenu(this);
    structMenu->setTitle(tr("Structure", "menubar"));
    menuBar()->addMenu(structMenu);

    structMenu->addAction(dbTree->getAction(DbTree::ADD_TABLE));
    structMenu->addAction(dbTree->getAction(DbTree::EDIT_TABLE));
    structMenu->addAction(dbTree->getAction(DbTree::DEL_TABLE));
    structMenu->addSeparator();
    structMenu->addAction(dbTree->getAction(DbTree::ADD_INDEX));
    structMenu->addAction(dbTree->getAction(DbTree::EDIT_INDEX));
    structMenu->addAction(dbTree->getAction(DbTree::DEL_INDEX));
    structMenu->addSeparator();
    structMenu->addAction(dbTree->getAction(DbTree::ADD_TRIGGER));
    structMenu->addAction(dbTree->getAction(DbTree::EDIT_TRIGGER));
    structMenu->addAction(dbTree->getAction(DbTree::DEL_TRIGGER));
    structMenu->addSeparator();
    structMenu->addAction(dbTree->getAction(DbTree::ADD_VIEW));
    structMenu->addAction(dbTree->getAction(DbTree::EDIT_VIEW));
    structMenu->addAction(dbTree->getAction(DbTree::DEL_VIEW));

    // View menu
    viewMenu = createPopupMenu();
    viewMenu->setTitle(tr("View", "menubar"));
    menuBar()->addMenu(viewMenu);

    mdiMenu = new QMenu(viewMenu);
    mdiMenu->setTitle(tr("Window list", "menubar view menu"));
    connect(ui->mdiArea, &MdiArea::windowListChanged, this, &MainWindow::refreshMdiWindows);

    viewMenu->addSeparator();
    viewMenu->addAction(actionMap[MDI_TILE]);
    viewMenu->addAction(actionMap[MDI_TILE_HORIZONTAL]);
    viewMenu->addAction(actionMap[MDI_TILE_VERTICAL]);
    viewMenu->addAction(actionMap[MDI_CASCADE]);
    viewMenu->addSeparator();
    viewMenu->addAction(actionMap[CLOSE_WINDOW]);
    viewMenu->addAction(actionMap[CLOSE_OTHER_WINDOWS]);
    viewMenu->addAction(actionMap[CLOSE_ALL_WINDOWS]);
    viewMenu->addSeparator();
    viewMenu->addAction(actionMap[RESTORE_WINDOW]);
    viewMenu->addAction(actionMap[RENAME_WINDOW]);

    viewMenu->addSeparator();
    viewMenu->addMenu(mdiMenu);

    // Tools menu
    toolsMenu = new QMenu(this);
    toolsMenu->setTitle(tr("Tools", "menubar"));
    menuBar()->addMenu(toolsMenu);

    toolsMenu->addAction(actionMap[OPEN_SQL_EDITOR]);
    toolsMenu->addAction(actionMap[OPEN_DDL_HISTORY]);
    toolsMenu->addAction(actionMap[OPEN_FUNCTION_EDITOR]);
    toolsMenu->addAction(actionMap[OPEN_COLLATION_EDITOR]);
    toolsMenu->addAction(actionMap[IMPORT]);
    toolsMenu->addAction(actionMap[EXPORT]);
    toolsMenu->addSeparator();
    toolsMenu->addAction(actionMap[OPEN_CONFIG]);

    // Help menu
    sqlitestudioMenu = new QMenu(this);
    sqlitestudioMenu->setTitle(tr("Help"));
    menuBar()->addMenu(sqlitestudioMenu);
    if (isDebugEnabled() && isDebugConsoleEnabled())
    {
        sqlitestudioMenu->addAction(actionMap[OPEN_DEBUG_CONSOLE]);
        sqlitestudioMenu->addSeparator();
    }
    sqlitestudioMenu->addAction(actionMap[USER_MANUAL]);
    sqlitestudioMenu->addAction(actionMap[SQLITE_DOCS]);
    sqlitestudioMenu->addAction(actionMap[HOMEPAGE]);
    sqlitestudioMenu->addAction(actionMap[FORUM]);
    sqlitestudioMenu->addSeparator();
    if (UPDATES->isPlatformEligibleForUpdate())
    {
        sqlitestudioMenu->addAction(actionMap[CHECK_FOR_UPDATES]);
        sqlitestudioMenu->addSeparator();
    }
    sqlitestudioMenu->addAction(actionMap[REPORT_BUG]);
    sqlitestudioMenu->addAction(actionMap[FEATURE_REQUEST]);
    sqlitestudioMenu->addAction(actionMap[BUG_REPORT_HISTORY]);
    sqlitestudioMenu->addSeparator();
    sqlitestudioMenu->addAction(actionMap[LICENSES]);
    sqlitestudioMenu->addAction(actionMap[ABOUT]);
}

void MainWindow::saveSession(MdiWindow* currWindow)
{
    /*
     * The currWindow is passed as parameter to the method to let hide the main window before
     * saving session, because saving might take a while.
     */
    QHash<QString,QVariant> sessionValue;
    sessionValue["state"] = saveState();
    sessionValue["geometry"] = saveGeometry();

    QList<QVariant> windowSessions;
    foreach (MdiWindow* window, ui->mdiArea->getWindows())
        if (window->restoreSessionNextTime())
            windowSessions << window->saveSession();

    sessionValue["windowSessions"] = windowSessions;

    if (currWindow && currWindow->restoreSessionNextTime())
    {
        QString title = currWindow->windowTitle();
        sessionValue["activeWindowTitle"] = title;
    }

    sessionValue["dbTree"] = dbTree->saveSession();
    sessionValue["style"] = currentStyle();

    CFG_UI.General.Session.set(sessionValue);
}

void MainWindow::restoreSession()
{
    QHash<QString,QVariant> sessionValue = CFG_UI.General.Session.get();
    if (sessionValue.size() == 0)
        return;

    if (sessionValue.contains("style"))
        setStyle(sessionValue["style"].toString());

    if (sessionValue.contains("geometry"))
        restoreGeometry(sessionValue["geometry"].toByteArray());

    if (sessionValue.contains("state"))
        restoreState(sessionValue["state"].toByteArray());

    if (sessionValue.contains("dbTree"))
        dbTree->restoreSession(sessionValue["dbTree"]);

    if (sessionValue.contains("windowSessions"))
        restoreWindowSessions(sessionValue["windowSessions"].toList());

    if (sessionValue.contains("activeWindowTitle"))
    {
        QString title = sessionValue["activeWindowTitle"].toString();
        MdiWindow* window = ui->mdiArea->getWindowByTitle(title);
        if (window)
            ui->mdiArea->setActiveSubWindow(window);
    }

    if (statusField->hasMessages())
        statusField->setVisible(true);

    updateWindowActions();
}

void MainWindow::restoreWindowSessions(const QList<QVariant>& windowSessions)
{
    if (windowSessions.size() == 0)
        return;

    foreach (const QVariant& winSession, windowSessions)
        restoreWindowSession(winSession);
}

MdiWindow* MainWindow::restoreWindowSession(const QVariant &windowSessions)
{
    QHash<QString, QVariant> winSessionHash = windowSessions.toHash();
    if (!winSessionHash.contains("class"))
        return nullptr;

    // Find out the type of stored session
    QByteArray classBytes = winSessionHash["class"].toString().toLatin1();
    char* className = classBytes.data();
    int type = QMetaType::type(className);
    if (type == QMetaType::UnknownType)
    {
        qWarning() << "Could not restore window session, because type" << className
                   << "is not known to Qt meta subsystem.";
        return nullptr;
    }

    // Try to instantiate the object
    void* object = QMetaType::create(type);
    if (!object)
    {
        qWarning() << "Could not restore window session, because type" << className
                   << "could not be instantiated.";
        return nullptr;
    }

    // Switch to session aware window, so we can use its session aware interface.
    MdiChild* mdiChild = reinterpret_cast<MdiChild*>(object);
    if (mdiChild->isInvalid())
    {
        delete mdiChild;
        return nullptr;
    }

    // Add the window to MDI area and restore its session
    MdiWindow* window = ui->mdiArea->addSubWindow(mdiChild);
    if (!window->restoreSession(winSessionHash))
        delete window;

    return window;
}

void MainWindow::setStyle(const QString& styleName)
{
    QStyle* style = QStyleFactory::create(styleName);
    if (!style)
    {
        notifyWarn(tr("Could not set style: %1", "main window").arg(styleName));
        return;
    }
    QApplication::setStyle(style);
}

QString MainWindow::currentStyle() const
{
    return QApplication::style()->objectName();
}

void MainWindow::closeNonSessionWindows()
{
    foreach (MdiWindow* window, ui->mdiArea->getWindows())
        if (!window->restoreSessionNextTime())
            window->close();
}
FormManager* MainWindow::getFormManager() const
{
    return formManager;
}

void MainWindow::setupDefShortcuts()
{
    BIND_SHORTCUTS(MainWindow, Action);
}

void MainWindow::openSqlEditorSlot()
{
    openSqlEditor();
}

void MainWindow::refreshMdiWindows()
{
    mdiMenu->clear();

    foreach (QAction* action, getMdiArea()->getTaskBar()->getTasks())
        mdiMenu->addAction(action);

    updateWindowActions();
}

void MainWindow::hideStatusField()
{
    statusField->close();
}

void MainWindow::openConfig()
{
    ConfigDialog config(this);
    config.exec();
}

void MainWindow::openDdlHistorySlot()
{
    openDdlHistory();
}

void MainWindow::openFunctionEditorSlot()
{
    openFunctionEditor();
}

void MainWindow::openCollationEditorSlot()
{
    openCollationEditor();
}

void MainWindow::exportAnything()
{
    if (!ExportManager::isAnyPluginAvailable())
    {
        notifyError(tr("Cannot export, because no export plugin is loaded."));
        return;
    }

    ExportDialog dialog(this);
    dialog.exec();
}

void MainWindow::importAnything()
{
    if (!ImportManager::isAnyPluginAvailable())
    {
        notifyError(tr("Cannot import, because no import plugin is loaded."));
        return;
    }

    ImportDialog dialog(this);
    dialog.exec();
}

void MainWindow::closeAllWindows()
{
    ui->mdiArea->closeAllSubWindows();
}

void MainWindow::closeAllWindowsButSelected()
{
    ui->mdiArea->closeAllButActive();
}

void MainWindow::closeSelectedWindow()
{
    ui->mdiArea->closeActiveSubWindow();
}

void MainWindow::renameWindow()
{
    MdiWindow* win = ui->mdiArea->getActiveWindow();
    if (!win)
        return;

    QString newTitle = QInputDialog::getText(this, tr("Rename window"), tr("Enter new name for the window:"), QLineEdit::Normal, win->windowTitle());
    if (newTitle == win->windowTitle() || newTitle.isEmpty())
        return;

    win->rename(newTitle);

}

void MainWindow::openDebugConsole()
{
    showUiDebugConsole();
}

void MainWindow::reportBug()
{
    BugDialog dialog(this);
    dialog.exec();
}

void MainWindow::requestFeature()
{
    BugDialog dialog(this);
    dialog.setFeatureRequestMode(true);
    dialog.exec();
}

void MainWindow::aboutSqlitestudio()
{
    AboutDialog dialog(AboutDialog::ABOUT, this);
    dialog.exec();
}

void MainWindow::licenses()
{
    AboutDialog dialog(AboutDialog::LICENSES, this);
    dialog.exec();
}

void MainWindow::homepage()
{
    QDesktopServices::openUrl(QUrl(SQLITESTUDIO->getHomePage()));
}

void MainWindow::forum()
{
    QDesktopServices::openUrl(QUrl(SQLITESTUDIO->getForumPage()));
}

void MainWindow::userManual()
{
    QDesktopServices::openUrl(QUrl(SQLITESTUDIO->getUserManualPage()));
}

void MainWindow::sqliteDocs()
{
    QDesktopServices::openUrl(QUrl(SQLITESTUDIO->getSqliteDocsPage()));
}

void MainWindow::reportHistory()
{
    openReportHistory();
}

void MainWindow::updatesAvailable(const QList<UpdateManager::UpdateEntry>& updates)
{
    manualUpdatesChecking = false;
    newVersionDialog = new NewVersionDialog(this);
    newVersionDialog->setUpdates(updates);
    notifyInfo(tr("New updates are available. <a href=\"%1\">Click here for details</a>.").arg(openUpdatesUrl));
}

void MainWindow::noUpdatesAvailable()
{
    if (!manualUpdatesChecking)
        return;

    notifyInfo(tr("You're running the most recent version. No updates are available."));
    manualUpdatesChecking = false;
}

void MainWindow::statusFieldLinkClicked(const QString& link)
{
    if (link == openUpdatesUrl && newVersionDialog)
    {
        newVersionDialog->exec();
        return;
    }
}

void MainWindow::checkForUpdates()
{
    manualUpdatesChecking = true;
    UPDATES->checkForUpdates();
}

void MainWindow::handleUpdatingProgress(const QString& jobTitle, int jobPercent, int totalPercent)
{
    if (!widgetCover->isVisible())
        widgetCover->show();

    updatingLabel->setText(jobTitle);
    updatingBusyBar->setValue(totalPercent);
    updatingSubBar->setValue(jobPercent);
}

void MainWindow::handleUpdatingError()
{
    widgetCover->hide();
}

DdlHistoryWindow* MainWindow::openDdlHistory()
{
    return openMdiWindow<DdlHistoryWindow>();
}

FunctionsEditor* MainWindow::openFunctionEditor()
{
    return openMdiWindow<FunctionsEditor>();
}

CollationsEditor* MainWindow::openCollationEditor()
{
    return openMdiWindow<CollationsEditor>();
}

BugReportHistoryWindow* MainWindow::openReportHistory()
{
    return openMdiWindow<BugReportHistoryWindow>();
}

bool MainWindow::confirmQuit(const QList<Committable*>& instances)
{
    QuitConfirmDialog dialog(MAINWINDOW);

    for (Committable* c : instances)
    {
        if (c->isUncommited())
            dialog.addMessage(c->getQuitUncommitedConfirmMessage());
    }

    if (dialog.getMessageCount() == 0)
        return true;

    if (dialog.exec() == QDialog::Accepted)
        return true;

    return false;
}

bool MainWindow::isClosingApp() const
{
    return closingApp;
}

QToolBar* MainWindow::getToolBar(int toolbar) const
{
    switch (static_cast<ToolBar>(toolbar))
    {
        case TOOLBAR_MAIN:
            return ui->mainToolBar;
        case TOOLBAR_DATABASE:
            return ui->dbToolbar;
        case TOOLBAR_STRUCTURE:
            return ui->structureToolbar;
        case TOOLBAR_VIEW:
            return ui->viewToolbar;
    }
    return nullptr;
}

void MainWindow::openDb(const QString& path)
{
    QString name = DBLIST->quickAddDb(path, QHash<QString,QVariant>());
    if (!name.isNull())
    {
        notifyInfo(tr("Database passed in command line parameters (%1) has been temporarily added to the list under name: %2").arg(path, name));
        Db* db = DBLIST->getByName(name);
        db->open();
    }
    else
        notifyError(tr("Could not add database %1 to list.").arg(path));
}

QMenu* MainWindow::getDatabaseMenu() const
{
    return dbMenu;
}

QMenu* MainWindow::getStructureMenu() const
{
    return structMenu;
}

QMenu* MainWindow::getViewMenu() const
{
    return viewMenu;
}

QMenu* MainWindow::getToolsMenu() const
{
    return toolsMenu;
}

QMenu* MainWindow::getSQLiteStudioMenu() const
{
    return sqlitestudioMenu;
}

MainWindow *MainWindow::getInstance()
{
    if (!instance)
        instance = new MainWindow();

    return instance;
}

bool MainWindow::eventFilter(QObject* obj, QEvent* e)
{
    UNUSED(obj);
    if (e->type() == QEvent::FileOpen)
    {
        QUrl url = dynamic_cast<QFileOpenEvent*>(e)->url();
        if (!url.isLocalFile())
            return false;

        DbDialog dialog(DbDialog::ADD, this);
        dialog.setPath(url.toLocalFile());
        dialog.exec();
        return true;
    }
    return false;
}

void MainWindow::pushClosedWindowSessionValue(const QVariant &value)
{
    closedWindowSessionValues.enqueue(value);

    if (closedWindowSessionValues.size() > closedWindowsStackSize)
        closedWindowSessionValues.dequeue();
}

void MainWindow::restoreLastClosedWindow()
{
    if (closedWindowSessionValues.size() == 0)
        return;

    QMdiSubWindow* activeWin = ui->mdiArea->activeSubWindow();
    bool maximizedMode = activeWin && activeWin->isMaximized();

    QVariant winSession = closedWindowSessionValues.takeLast();
    if (maximizedMode)
    {
        QHash<QString, QVariant> winSessionHash = winSession.toHash();
        winSessionHash.remove("geometry");
        winSession = winSessionHash;
    }

    restoreWindowSession(winSession);
}

bool MainWindow::hasClosedWindowToRestore() const
{
    return closedWindowSessionValues.size() > 0;
}
