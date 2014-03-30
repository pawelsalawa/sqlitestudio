#include "mainwindow.h"
#include "dbtree/dbtree.h"
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
#include <QMdiSubWindow>
#include <QDebug>
#include <QStyleFactory>
#include <QUiLoader>

MainWindow* MainWindow::instance = nullptr;

MainWindow::MainWindow() :
    QMainWindow(),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    init();
}

MainWindow::~MainWindow()
{
    delete ui;

    if (formManager)
    {
        delete formManager;
        formManager = nullptr;
    }
}

void MainWindow::init()
{
    setWindowIcon(ICONS.SQLITESTUDIO_APP);

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
    PLUGINS->loadBuiltInPlugin(new SqliteHighlighterPlugin);
    PLUGINS->loadBuiltInPlugin(new JavaScriptHighlighterPlugin);
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

QAction *MainWindow::getAction(MainWindow::Action action)
{
    Q_ASSERT(actionMap.contains(action));
    return actionMap.value(action);
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
    createAction(OPEN_CONFIG, ICONS.CONFIGURE, tr("Open configuration dialog"), this, SLOT(openConfig()), ui->mainToolBar);
    createAction(MDI_TILE, ICONS.WIN_TILE, tr("Tile windows"), ui->mdiArea, SLOT(tileSubWindows()), ui->viewToolbar);
    createAction(MDI_TILE_HORIZONTAL, ICONS.WIN_TILE_HORIZONTAL, tr("Tile windows horizontally"), ui->mdiArea, SLOT(tileHorizontally()), ui->viewToolbar);
    createAction(MDI_TILE_VERTICAL, ICONS.WIN_TILE_VERTICAL, tr("Tile windows vertically"), ui->mdiArea, SLOT(tileVertically()), ui->viewToolbar);
    createAction(MDI_CASCADE, ICONS.WIN_CASCADE, tr("Cascade windows"), ui->mdiArea, SLOT(cascadeSubWindows()), ui->viewToolbar);
    createAction(NEXT_TASK, tr("Next window"), ui->taskBar, SLOT(nextTask()), this);
    createAction(PREV_TASK, tr("Previous window"), ui->taskBar, SLOT(prevTask()), this);
    createAction(HIDE_STATUS_FIELD, tr("Hide status field"), this, SLOT(hideStatusField()), this);

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
}

void MainWindow::initMenuBar()
{
    // Database menu
    QMenu* dbMenu = new QMenu(this);
    dbMenu->setTitle(tr("Database", "menubar"));
    menuBar()->addMenu(dbMenu);

    dbMenu->addAction(dbTree->getAction(DbTree::CONNECT_TO_DB));
    dbMenu->addAction(dbTree->getAction(DbTree::DISCONNECT_FROM_DB));
    dbMenu->addSeparator();
    dbMenu->addAction(dbTree->getAction(DbTree::ADD_DB));
    dbMenu->addAction(dbTree->getAction(DbTree::EDIT_DB));
    dbMenu->addAction(dbTree->getAction(DbTree::DELETE_DB));
    dbMenu->addSeparator();
    dbMenu->addAction(dbTree->getAction(DbTree::REFRESH_SCHEMA));
    dbMenu->addAction(dbTree->getAction(DbTree::REFRESH_SCHEMAS));

    // Structure menu
    QMenu* structMenu = new QMenu(this);
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
    QMenu* viewMenu = createPopupMenu();
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
    viewMenu->addMenu(mdiMenu);
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
}

void MainWindow::restoreWindowSessions(const QList<QVariant>& windowSessions)
{
    if (windowSessions.size() == 0)
        return;

    int type;
    char* className = nullptr;
    void* object = nullptr;
    MdiChild* mdiChild = nullptr;
    MdiWindow* window = nullptr;
    QByteArray classBytes;
    QHash<QString, QVariant> winSessionHash;
    foreach (const QVariant& winSession, windowSessions)
    {
        winSessionHash = winSession.toHash();
        if (!winSessionHash.contains("class"))
            continue;

        // Find out the type of stored session
        classBytes = winSessionHash["class"].toString().toLatin1();
        className = classBytes.data();
        type = QMetaType::type(className);
        if (type == QMetaType::UnknownType)
        {
            qWarning() << "Could not restore window session, because type" << className
                       << "is not known to Qt meta subsystem.";
            continue;
        }

        // Try to instantiate the object
        object = QMetaType::create(type);
        if (!object)
        {
            qWarning() << "Could not restore window session, because type" << className
                       << "could not be instantiated.";
            continue;
        }

        // Switch to session aware window, so we can use its session aware interface.
        mdiChild = reinterpret_cast<MdiChild*>(object);
        if (mdiChild->isInvalid())
        {
            delete window;
            continue;
        }

        // Add the window to MDI area and restore its session
        window = ui->mdiArea->addSubWindow(mdiChild);
        if (!window->restoreSession(winSessionHash))
        {
            delete window;
            continue;
        }
    }
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
    defShortcut(OPEN_SQL_EDITOR, Qt::ALT + Qt::Key_E);
    defShortcut(PREV_TASK, Qt::CTRL + Qt::Key_PageUp);
    defShortcut(NEXT_TASK, Qt::CTRL + Qt::Key_PageDown);
    defShortcut(HIDE_STATUS_FIELD, Qt::Key_Escape);
    defShortcut(OPEN_CONFIG, Qt::Key_F2);
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

DdlHistoryWindow* MainWindow::openDdlHistory()
{
    return openMdiWindow<DdlHistoryWindow>();
//    DdlHistoryWindow* win = nullptr;
//    foreach (MdiWindow* mdiWin, ui->mdiArea->getWindows())
//    {
//        win = dynamic_cast<DdlHistoryWindow*>(mdiWin->getMdiChild());
//        if (win)
//        {
//            ui->mdiArea->setActiveSubWindow(mdiWin);
//            return win;
//        }
//    }

//    win = new DdlHistoryWindow(ui->mdiArea);
//    if (win->isInvalid())
//    {
//        delete win;
//        return nullptr;
//    }

//    ui->mdiArea->addSubWindow(win);
//    return win;
}

FunctionsEditor* MainWindow::openFunctionEditor()
{
    return openMdiWindow<FunctionsEditor>();
}

CollationsEditor* MainWindow::openCollationEditor()
{
    return openMdiWindow<CollationsEditor>();
}

MainWindow *MainWindow::getInstance()
{
    if (!instance)
        instance = new MainWindow();

    return instance;
}
