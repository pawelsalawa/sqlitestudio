#include "mainwindow.h"
#include "common/unused.h"
#include "dbtree/dbtree.h"
#include "dbtree/dbtreemodel.h"
#include "iconmanager.h"
#include "windows/editorwindow.h"
#include "windows/functionseditor.h"
#include "windows/collationseditor.h"
#include "windows/ddlhistorywindow.h"
#include "windows/sqliteextensioneditor.h"
#include "mdiarea.h"
#include "statusfield.h"
#include "uiconfig.h"
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
#include "dialogs/newversiondialog.h"
#include "dialogs/quitconfirmdialog.h"
#include "dialogs/cssdebugdialog.h"
#include "themetuner.h"
#include "style.h"
#include "services/codeformatter.h"
#include "windows/codesnippeteditor.h"
#include "uiutils.h"
#include <QMdiSubWindow>
#include <QDebug>
#include <QStyleFactory>
#include <QUiLoader>
#include <QInputDialog>
#include <QProgressBar>
#include <QLabel>
#include <QStyle>
#include <QApplication>
#include <QToolTip>
#include <QTimer>
#include <QtGui>

CFG_KEYS_DEFINE(MainWindow)
MainWindow* MainWindow::instance = nullptr;
bool MainWindow::safeModeEnabled = false;

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
    connect(SQLITESTUDIO, SIGNAL(aboutToQuit()), this, SLOT(cleanUp()));

#ifdef Q_OS_WIN
    setWindowIcon(ICONS.SQLITESTUDIO_APP.toQIcon().pixmap(256, 256));
#else
    setWindowIcon(ICONS.SQLITESTUDIO_APP);
#endif

    setWindowTitle(QString("SQLiteStudio (%1)").arg(SQLITESTUDIO->getVersionString()));

#ifdef Q_OS_MACX
    ui->centralWidget->layout()->setContentsMargins(0, 0, 0, 0);
#endif

    Committable::init(MainWindow::confirmQuit);
    updateCornerDocking();

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

    ui->viewToolbar->setIconSize(QSize(24, 24));
    ui->mainToolBar->setIconSize(QSize(24, 24));
    ui->structureToolbar->setIconSize(QSize(24, 24));
    ui->dbToolbar->setIconSize(QSize(24, 24));

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

#ifdef PORTABLE_CONFIG
    connect(UPDATES, SIGNAL(updateAvailable(QString, QString)), this, SLOT(updateAvailable(QString, QString)));
    connect(UPDATES, SIGNAL(noUpdatesAvailable()), this, SLOT(noUpdatesAvailable()));
#endif
    connect(statusField, SIGNAL(linkActivated(QString)), this, SLOT(statusFieldLinkClicked(QString)));

    connect(CFG_CORE.General.Language, SIGNAL(changed(QVariant)), this, SLOT(notifyAboutLanguageChange()));
    connect(CFG_UI.General.AllowMultipleSessions, SIGNAL(changed(QVariant)), this, SLOT(updateMultipleSessionsSetting(QVariant)));

    updateMultipleSessionsSetting();
    fixFonts();
    fixToolbars();
    observeSessionChanges();

    SQLITESTUDIO->installCrashHandler([this]()
    {
        saveSession();
    });
}

void MainWindow::observeSessionChanges()
{
    saveSessionTimer = new QTimer(this);
    saveSessionTimer->setSingleShot(true);
    connect(saveSessionTimer, SIGNAL(timeout()), this, SLOT(saveSession()));

    for (QDockWidget* dock : QList<QDockWidget*>({dbTree, statusField}))
    {
        connect(dock, SIGNAL(topLevelChanged(bool)), this, SLOT(scheduleSessionSave()));
        connect(dock, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), this, SLOT(scheduleSessionSave()));
        connect(dock, SIGNAL(visibilityChanged(bool)), this, SLOT(scheduleSessionSave()));
    }
    connect(dbTree, SIGNAL(sessionValueChanged()), this, SLOT(scheduleSessionSave()));
    connect(getMdiArea(), SIGNAL(sessionValueChanged()), this, SLOT(scheduleSessionSave()));
    connect(this, SIGNAL(sessionValueChanged()), this, SLOT(scheduleSessionSave()));
}

void MainWindow::cleanUp()
{
    if (SQLITESTUDIO->getImmediateQuit())
        return;

//    qDebug() << "MainWindow::cleanUp()";
    for (MdiWindow* win : getMdiArea()->getWindows())
        delete win;

    removeDockWidget(dbTree);
    removeDockWidget(statusField);

    safe_delete(dbTree);
    safe_delete(statusField);

    delete ui;

    ThemeTuner::cleanUp();

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
    actionMap[CLOSE_ALL_WINDOWS_LEFT]->setEnabled(hasActiveTask);
    actionMap[CLOSE_ALL_WINDOWS_RIGHT]->setEnabled(hasActiveTask);
    actionMap[RENAME_WINDOW]->setEnabled(hasActiveTask);
    actionMap[RESTORE_WINDOW]->setEnabled(hasClosedWindowToRestore());
}

void MainWindow::notifyAboutLanguageChange()
{
    notifyInfo(tr("You need to restart application to make the language change take effect."));
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

    saveSessionTimer->stop();
    safe_delete(saveSessionTimer);

    closingApp = true;
    closeNonSessionWindows();
    saveSession(true);
    SQLITESTUDIO->cleanUp();
    QMainWindow::closeEvent(event);
}

void MainWindow::createActions()
{
    createAction(OPEN_SQL_EDITOR, ICONS.OPEN_SQL_EDITOR, tr("Open SQL &editor"), this, SLOT(openSqlEditorSlot()), ui->mainToolBar);
    createAction(OPEN_DDL_HISTORY, ICONS.DDL_HISTORY, tr("Open DDL &history"), this, SLOT(openDdlHistorySlot()), ui->mainToolBar);
    createAction(OPEN_FUNCTION_EDITOR, ICONS.FUNCTIONS_EDITOR, tr("Open SQL &functions editor"), this, SLOT(openFunctionEditorSlot()), ui->mainToolBar);
    createAction(OPEN_SNIPPETS_EDITOR, ICONS.CODE_SNIPPETS, tr("Open code &snippets editor"), this, SLOT(openCodeSnippetsEditorSlot()), ui->mainToolBar);
    createAction(OPEN_COLLATION_EDITOR, ICONS.COLLATIONS_EDITOR, tr("Open &collations editor"), this, SLOT(openCollationEditorSlot()), ui->mainToolBar);
    createAction(OPEN_EXTENSION_MANAGER, ICONS.EXTENSION_EDITOR, tr("Open ex&tension manager"), this, SLOT(openExtensionManagerSlot()), ui->mainToolBar);
    createAction(IMPORT, ICONS.IMPORT, tr("&Import"), this, SLOT(importAnything()), ui->mainToolBar);
    createAction(EXPORT, ICONS.EXPORT, tr("E&xport"), this, SLOT(exportAnything()), ui->mainToolBar);
    ui->mainToolBar->addSeparator();
    createAction(OPEN_CONFIG, ICONS.CONFIGURE, tr("Open confi&guration dialog"), this, SLOT(openConfig()), ui->mainToolBar);

    createAction(MDI_TILE, ICONS.WIN_TILE, tr("&Tile windows"), ui->mdiArea, SLOT(tileSubWindows()), ui->viewToolbar);
    createAction(MDI_TILE_HORIZONTAL, ICONS.WIN_TILE_HORIZONTAL, tr("Tile windows &horizontally"), ui->mdiArea, SLOT(tileHorizontally()), ui->viewToolbar);
    createAction(MDI_TILE_VERTICAL, ICONS.WIN_TILE_VERTICAL, tr("Tile windows &vertically"), ui->mdiArea, SLOT(tileVertically()), ui->viewToolbar);
    createAction(MDI_CASCADE, ICONS.WIN_CASCADE, tr("&Cascade windows"), ui->mdiArea, SLOT(cascadeSubWindows()), ui->viewToolbar);
    createAction(NEXT_TASK, tr("Next window"), ui->taskBar, SLOT(nextTask()), this);
    createAction(PREV_TASK, tr("Previous window"), ui->taskBar, SLOT(prevTask()), this);
    createAction(HIDE_STATUS_FIELD, tr("Hide status field"), this, SLOT(hideStatusField()), this);

    createAction(CLOSE_WINDOW, ICONS.WIN_CLOSE, tr("Close current &window"), this, SLOT(closeSelectedWindow()), this);
    createAction(CLOSE_OTHER_WINDOWS, ICONS.WIN_CLOSE_OTHER, tr("Close &other windows"), this, SLOT(closeAllWindowsButSelected()), this);
    createAction(CLOSE_ALL_WINDOWS, ICONS.WIN_CLOSE_ALL, tr("Close &all windows"), this, SLOT(closeAllWindows()), this);
    createAction(CLOSE_ALL_WINDOWS_LEFT, ICONS.WIN_CLOSE_ALL_LEFT, tr("Close windows on the &left"), this, SLOT(closeAllLeftWindows()), this);
    createAction(CLOSE_ALL_WINDOWS_RIGHT, ICONS.WIN_CLOSE_ALL_RIGHT, tr("Close windows on the &right"), this, SLOT(closeAllRightWindows()), this);
    createAction(RESTORE_WINDOW, ICONS.WIN_RESTORE, tr("Re&store recently closed window"), this, SLOT(restoreLastClosedWindow()), this);
    createAction(RENAME_WINDOW, ICONS.WIN_RENAME, tr("Re&name selected window"), this, SLOT(renameWindow()), this);

    createAction(OPEN_DEBUG_CONSOLE, tr("Open Debug Console"), this, SLOT(openDebugConsole()), this);
    createAction(OPEN_CSS_CONSOLE, tr("Open CSS Console"), this, SLOT(openCssConsole()), this);
    createAction(REPORT_BUG, ICONS.BUG, tr("Report a &bug"), this, SLOT(reportBug()), this);
    createAction(DONATE, ICONS.DONATE, tr("D&onate"), this, SLOT(donate()), this);
    createAction(FEATURE_REQUEST, ICONS.FEATURE_REQUEST, tr("Propose a new &feature"), this, SLOT(requestFeature()), this);
    createAction(ABOUT, ICONS.SQLITESTUDIO_APP16, tr("&About"), this, SLOT(aboutSqlitestudio()), this);
    createAction(LICENSES, ICONS.LICENSES, tr("&Licenses"), this, SLOT(licenses()), this);
    createAction(HOMEPAGE, ICONS.HOMEPAGE, tr("Open home &page"), this, SLOT(homepage()), this);
    createAction(USER_MANUAL, ICONS.USER_MANUAL, tr("User &Manual"), this, SLOT(userManual()), this);
    createAction(SQLITE_DOCS, ICONS.SQLITE_DOCS, tr("SQLite &documentation"), this, SLOT(sqliteDocs()), this);
    createAction(BUG_REPORT_HISTORY, ICONS.BUG_LIST, tr("Bugs and feature &requests"), this, SLOT(reportHistory()), this);
    createAction(QUIT, ICONS.QUIT, tr("Quit"), this, SLOT(quit()), this);
#ifdef PORTABLE_CONFIG
    createAction(CHECK_FOR_UPDATES, ICONS.GET_UPDATE, tr("Check for &updates"), this, SLOT(checkForUpdates()), this);
#endif

    actionMap[ABOUT]->setMenuRole(QAction::AboutRole);
    actionMap[OPEN_CONFIG]->setMenuRole(QAction::PreferencesRole);

    ui->dbToolbar->addAction(dbTree->getAction(DbTree::CONNECT_TO_DB));
    ui->dbToolbar->addAction(dbTree->getAction(DbTree::DISCONNECT_FROM_DB));
    ui->dbToolbar->addSeparator();
    ui->dbToolbar->addAction(dbTree->getAction(DbTree::ADD_DB));
    ui->dbToolbar->addAction(dbTree->getAction(DbTree::REFRESH_SCHEMA));

    ui->structureToolbar->addAction(dbTree->getAction(DbTree::ADD_TABLE));
    ui->structureToolbar->addAction(dbTree->getAction(DbTree::ADD_VIEW));

    ui->taskBar->initContextMenu(this);
}

void MainWindow::initMenuBar()
{
    // Database menu
    dbMenu = new QMenu(this);
    dbMenu->setTitle(tr("&Database", "menubar"));
    menuBar()->addMenu(dbMenu);

    dbMenu->addAction(dbTree->getAction(DbTree::CONNECT_TO_DB));
    dbMenu->addAction(dbTree->getAction(DbTree::DISCONNECT_FROM_DB));
    dbMenu->addSeparator();
    dbMenu->addAction(dbTree->getAction(DbTree::ADD_DB));
    dbMenu->addAction(dbTree->getAction(DbTree::EDIT_DB));
    dbMenu->addAction(dbTree->getAction(DbTree::DELETE_DB));
    dbMenu->addSeparator();
    dbMenu->addAction(dbTree->getAction(DbTree::EXPORT_DB));
    dbMenu->addAction(dbTree->getAction(DbTree::VACUUM_DB));
    dbMenu->addAction(dbTree->getAction(DbTree::INTEGRITY_CHECK));
    dbMenu->addSeparator();
    dbMenu->addAction(dbTree->getAction(DbTree::REFRESH_SCHEMA));
    dbMenu->addAction(dbTree->getAction(DbTree::REFRESH_SCHEMAS));
#ifndef Q_OS_MACX
    dbMenu->addSeparator();
    dbMenu->addAction(actionMap[QUIT]);
#endif

    // Structure menu
    structMenu = new QMenu(this);
    structMenu->setTitle(tr("&Structure", "menubar"));
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
    viewMenu->setTitle(tr("&View", "menubar"));
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
    viewMenu->addAction(actionMap[CLOSE_ALL_WINDOWS]);
    viewMenu->addAction(actionMap[CLOSE_OTHER_WINDOWS]);
    viewMenu->addAction(actionMap[CLOSE_ALL_WINDOWS_LEFT]);
    viewMenu->addAction(actionMap[CLOSE_ALL_WINDOWS_RIGHT]);
    viewMenu->addSeparator();
    viewMenu->addAction(actionMap[RESTORE_WINDOW]);
    viewMenu->addAction(actionMap[RENAME_WINDOW]);

    viewMenu->addSeparator();
    viewMenu->addMenu(mdiMenu);

    // Tools menu
    toolsMenu = new QMenu(this);
    toolsMenu->setTitle(tr("&Tools", "menubar"));
    menuBar()->addMenu(toolsMenu);

    toolsMenu->addAction(actionMap[OPEN_SQL_EDITOR]);
    toolsMenu->addAction(actionMap[OPEN_DDL_HISTORY]);
    toolsMenu->addAction(actionMap[OPEN_FUNCTION_EDITOR]);
    toolsMenu->addAction(actionMap[OPEN_SNIPPETS_EDITOR]);
    toolsMenu->addAction(actionMap[OPEN_COLLATION_EDITOR]);
    toolsMenu->addAction(actionMap[OPEN_EXTENSION_MANAGER]);
    toolsMenu->addAction(actionMap[IMPORT]);
    toolsMenu->addAction(actionMap[EXPORT]);
    toolsMenu->addSeparator();
    toolsMenu->addAction(actionMap[OPEN_CONFIG]);

    // Help menu
    sqlitestudioMenu = new QMenu(this);
    sqlitestudioMenu->setTitle(tr("&Help"));
    menuBar()->addMenu(sqlitestudioMenu);
    if (isDebugEnabled() && isDebugConsoleEnabled())
    {
        sqlitestudioMenu->addAction(actionMap[OPEN_DEBUG_CONSOLE]);
        sqlitestudioMenu->addSeparator();
    }
    sqlitestudioMenu->addAction(actionMap[USER_MANUAL]);
    sqlitestudioMenu->addAction(actionMap[SQLITE_DOCS]);
    sqlitestudioMenu->addAction(actionMap[HOMEPAGE]);
    sqlitestudioMenu->addSeparator();
#ifdef PORTABLE_CONFIG
    if (UPDATES->isPlatformEligibleForUpdate())
    {
        sqlitestudioMenu->addAction(actionMap[CHECK_FOR_UPDATES]);
        sqlitestudioMenu->addSeparator();
    }
#endif
    sqlitestudioMenu->addAction(actionMap[REPORT_BUG]);
    sqlitestudioMenu->addAction(actionMap[FEATURE_REQUEST]);
    sqlitestudioMenu->addAction(actionMap[BUG_REPORT_HISTORY]);
    sqlitestudioMenu->addSeparator();
    sqlitestudioMenu->addAction(actionMap[LICENSES]);
    sqlitestudioMenu->addAction(actionMap[DONATE]);
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

    if (CFG_UI.General.RestoreSession.get())
    {
        QList<QVariant> windowSessions;
        for (MdiWindow* window : ui->mdiArea->getWindows())
            if (window->restoreSessionNextTime())
                windowSessions << window->saveSession();

        sessionValue["windowSessions"] = windowSessions;

        if (currWindow && currWindow->restoreSessionNextTime())
        {
            QString title = currWindow->windowTitle();
            sessionValue["activeWindowTitle"] = title;
        }
    }

    sessionValue["dbTree"] = dbTree->saveSession();
    sessionValue["style"] = currentStyle();

    CFG_UI.General.Session.set(sessionValue);
}

void MainWindow::restoreSession()
{
    if (safeModeEnabled)
    {
        qInfo() << "Safe-Mode active. Skipping last saved session.";
        return;
    }

    QHash<QString,QVariant> sessionValue = CFG_UI.General.Session.get();
    if (sessionValue.size() == 0)
    {
        THEME_TUNER->tuneCurrentTheme();
        restoreState(saveState()); // workaround for probable Qt bug (?), reported in #3421
        return;
    }

    if (sessionValue.contains("style"))
    {
        QString styleName = sessionValue["style"].toString();
        setStyle(styleName);
    }
    else
        THEME_TUNER->tuneCurrentTheme();

    QString styleName = currentStyle();
    CFG_UI.General.Style.set(styleName);

    if (sessionValue.contains("geometry"))
        restoreGeometry(sessionValue["geometry"].toByteArray());

    if (sessionValue.contains("state"))
        restoreState(sessionValue["state"].toByteArray());
    else
        restoreState(saveState()); // workaround for probable Qt bug (?), reported in #3421

    if (sessionValue.contains("dbTree"))
        dbTree->restoreSession(sessionValue["dbTree"]);

    if (CFG_UI.General.RestoreSession.get())
    {
        if (sessionValue.contains("windowSessions"))
            restoreWindowSessions(sessionValue["windowSessions"].toList());

        if (sessionValue.contains("activeWindowTitle"))
        {
            QString title = sessionValue["activeWindowTitle"].toString();
            MdiWindow* window = ui->mdiArea->getWindowByTitle(title);
            if (window)
                ui->mdiArea->setActiveSubWindow(window);
        }
    }

    if (statusField->hasMessages())
        statusField->setVisible(true);

    updateCornerDocking();
    updateWindowActions();
}

void MainWindow::restoreWindowSessions(const QList<QVariant>& windowSessions)
{
    if (windowSessions.size() == 0)
        return;

    for (const QVariant& winSession : windowSessions)
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
    QMetaType metaType = QMetaType::fromName(className);
    if (!metaType.isValid())
    {
        qWarning() << "Could not restore window session, because type" << className
                   << "is not known to Qt meta subsystem.";
        return nullptr;
    }

    // Try to instantiate the object
    void* object = metaType.create();
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
    {
        window->setCloseWithoutSessionSaving(true);
        delete window;
        return nullptr;
    }

    return window;
}

bool MainWindow::setStyle(const QString& styleName)
{
    QStyle* style = QStyleFactory::create(styleName);
    if (!style)
    {
        notifyWarn(tr("Could not set style: %1", "main window").arg(styleName));
        return false;
    }

    STYLE->setStyle(style, styleName);
    statusField->refreshColors();
    return true;
}

QString MainWindow::currentStyle() const
{
    return STYLE->name();
}

EditorWindow* MainWindow::openSqlEditor(Db* dbToSet, const QString& sql)
{
    EditorWindow* win = openSqlEditor();
    if (!win->setCurrentDb(dbToSet))
    {
        qCritical() << "Created EditorWindow had not got requested database:" << dbToSet->getName();
        win->close();
        return nullptr;
    }

    win->setContents(FORMATTER->format("sql", sql, dbToSet));
    return win;
}

void MainWindow::saveSession(bool hide)
{
    MdiWindow* currWindow = ui->mdiArea->getCurrentWindow();
    if (hide)
        this->hide();

    saveSession(currWindow);
}

void MainWindow::saveSession()
{
    saveSession(false);
}

void MainWindow::scheduleSessionSave()
{
    if (saveSessionTimer)
        saveSessionTimer->start(saveSessionDelayMs);
}

void MainWindow::closeNonSessionWindows()
{
    for (MdiWindow* window : ui->mdiArea->getWindows())
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

    QStringList actionNames;
    QHash<QString, QAction*> nameToAction;
    for (QAction* action : getMdiArea()->getTaskBar()->getTasks())
    {
        actionNames << action->text();
        nameToAction[action->text()] = action;
    }

    sSort(actionNames);

    for (const QString& name : actionNames)
        mdiMenu->addAction(nameToAction[name]);
    fixToolbarTooltips(ui->viewToolbar);

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

void MainWindow::openCodeSnippetsEditorSlot()
{
    openCodeSnippetEditor();
}

void MainWindow::openCollationEditorSlot()
{
    openCollationEditor();
}

void MainWindow::openExtensionManagerSlot()
{
    openExtensionManager();
}

void MainWindow::exportAnything()
{
    if (!ExportManager::isAnyPluginAvailable())
    {
        notifyError(tr("Cannot export, because no export plugin is loaded."));
        return;
    }

    ExportDialog dialog(this);
    Db* db = DBTREE->getSelectedOpenDb();
    if (db)
        dialog.setPreselectedDb(db);

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
    Db* db = DBTREE->getSelectedOpenDb();
    if (db)
        dialog.setDb(db);

    dialog.exec();
}

void MainWindow::closeAllWindows()
{
    ui->mdiArea->closeAllSubWindows();
}

void MainWindow::closeAllLeftWindows()
{
    ui->mdiArea->closeAllLeftToActive();
}

void MainWindow::closeAllRightWindows()
{
    ui->mdiArea->closeAllRightToActive();
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

void MainWindow::openCssConsole()
{
    CssDebugDialog* dialog = new CssDebugDialog;
    dialog->show();
}

void MainWindow::reportBug()
{
    QDesktopServices::openUrl(QUrl(SQLITESTUDIO->getNewIssuePage()));
}

void MainWindow::requestFeature()
{
    QDesktopServices::openUrl(QUrl(SQLITESTUDIO->getNewIssuePage()));
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

void MainWindow::githubReleases()
{
    QDesktopServices::openUrl(QUrl(SQLITESTUDIO->getGitHubReleases()));
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
    QDesktopServices::openUrl(QUrl(SQLITESTUDIO->getIssuesPage()));
}

void MainWindow::donate()
{
    QDesktopServices::openUrl(QUrl(SQLITESTUDIO->getDonatePage()));
}

void MainWindow::statusFieldLinkClicked(const QString& link)
{
#ifdef PORTABLE_CONFIG
    if (link == openUpdatesUrl && newVersionDialog)
    {
        newVersionDialog->exec();
        return;
    }
#endif
}

void MainWindow::quit()
{
    close();
}

void MainWindow::updateMultipleSessionsSetting(const QVariant& newValue)
{
    Config::getSettings()->setValue(ALLOW_MULTIPLE_SESSIONS_SETTING, newValue);
}

void MainWindow::updateMultipleSessionsSetting()
{
    Config::getSettings()->setValue(ALLOW_MULTIPLE_SESSIONS_SETTING, CFG_UI.General.AllowMultipleSessions.get());
}

#ifdef PORTABLE_CONFIG
void MainWindow::updateAvailable(const QString& version, const QString& url)
{
    manualUpdatesChecking = false;
    newVersionDialog = new NewVersionDialog(this);
    newVersionDialog->setUpdate(version, url);
    notifyInfo(tr("New updates are available. <a href=\"%1\">Click here for details</a>.").arg(openUpdatesUrl));
}

void MainWindow::noUpdatesAvailable()
{
    if (!manualUpdatesChecking)
        return;

    notifyInfo(tr("You're running the most recent version. No updates are available."));
    manualUpdatesChecking = false;
}

void MainWindow::checkForUpdates()
{
    manualUpdatesChecking = true;
    UPDATES->checkForUpdates();
}

#endif

void MainWindow::updateCornerDocking()
{
    if (CFG_UI.General.DockLayout.get() == "vertical") {
        setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
        setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
        setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
        setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);
    } else {
        setCorner(Qt::TopLeftCorner, Qt::TopDockWidgetArea);
        setCorner(Qt::TopRightCorner, Qt::TopDockWidgetArea);
        setCorner(Qt::BottomLeftCorner, Qt::BottomDockWidgetArea);
        setCorner(Qt::BottomRightCorner, Qt::BottomDockWidgetArea);
    }
}

void MainWindow::messageFromSecondaryInstance(quint32 instanceId, QByteArray message)
{
    UNUSED(instanceId);
    activateWindow();
    if (isMinimized())
        showMaximized();

    raise();
    QString dbToOpen = deserializeFromBytes(message).toString();
    if (!dbToOpen.isNull())
        openDb(dbToOpen);
}

DdlHistoryWindow* MainWindow::openDdlHistory()
{
    return openMdiWindow<DdlHistoryWindow>();
}

FunctionsEditor* MainWindow::openFunctionEditor()
{
    return openMdiWindow<FunctionsEditor>();
}

CodeSnippetEditor* MainWindow::openCodeSnippetEditor()
{
    return openMdiWindow<CodeSnippetEditor>();
}

CollationsEditor* MainWindow::openCollationEditor()
{
    return openMdiWindow<CollationsEditor>();
}

SqliteExtensionEditor* MainWindow::openExtensionManager()
{
    return openMdiWindow<SqliteExtensionEditor>();
}

void MainWindow::fixFonts()
{
    CfgTypedEntry<QFont>* typed = nullptr;
    for (CfgEntry* cfg : CFG_UI.Fonts.getEntries())
    {
        typed = dynamic_cast<CfgTypedEntry<QFont>*>(cfg);
        if (typed->get().pointSize() == 0)
            cfg->set(cfg->getDefaultValue());
    }
}

void MainWindow::fixToolbars()
{
    fixToolbarTooltips(ui->viewToolbar);
    fixToolbarTooltips(ui->mainToolBar);
    fixToolbarTooltips(ui->structureToolbar);
    fixToolbarTooltips(ui->dbToolbar);
}

bool MainWindow::confirmQuit(const QList<Committable*>& instances)
{
    QuitConfirmDialog dialog(MAINWINDOW);

    for (Committable* c : instances)
    {
        if (c->isUncommitted())
            dialog.addMessage(c->getQuitUncommittedConfirmMessage());
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
    Db* db = DBLIST->getByPath(path);
    if (db)
    {
        notifyInfo(tr("Database passed in command line parameters (%1) was already on the list under name: %2").arg(path, db->getName()));
        return;
    }

    QString name = DBLIST->quickAddDb(path, QHash<QString,QVariant>());
    if (!name.isNull())
    {
        notifyInfo(tr("Database passed in command line parameters (%1) has been temporarily added to the list under name: %2").arg(path, name));
        db = DBLIST->getByName(name);
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

void MainWindow::setSafeMode(bool enabled)
{
    safeModeEnabled = enabled;
}

bool MainWindow::isSafeMode()
{
    return safeModeEnabled;
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
