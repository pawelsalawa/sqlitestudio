#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "common/extactioncontainer.h"
#include "db/db.h"
#include "ui_mainwindow.h"
#include "mdiwindow.h"
#include "guiSQLiteStudio_global.h"
#include <QMainWindow>
#include <QHash>
#include <QQueue>

class QUiLoader;
class DbTree;
class StatusField;
class EditorWindow;
class MdiArea;
class QActionGroup;
class MdiWindow;
class ViewWindow;
class TableWindow;
class FormManager;
class DdlHistoryWindow;
class FunctionsEditor;
class CollationsEditor;
class BugReportHistoryWindow;
class NewVersionDialog;
class Committable;
class WidgetCover;
class QProgressBar;
class QLabel;
class QTimer;
class ThemeTuner;
class SqliteExtensionEditor;
class CodeSnippetEditor;

#ifdef Q_OS_MACX
#define PREV_TASK_KEY_SEQ Qt::CTRL | Qt::ALT | Qt::Key_Left
#define NEXT_TASK_KEY_SEQ Qt::CTRL | Qt::ALT | Qt::Key_Right
#else
#define PREV_TASK_KEY_SEQ Qt::CTRL | Qt::Key_PageUp
#define NEXT_TASK_KEY_SEQ Qt::CTRL | Qt::Key_PageDown
#endif

CFG_KEY_LIST(MainWindow, QObject::tr("Main window"),
    CFG_KEY_ENTRY(OPEN_SQL_EDITOR,        Qt::ALT | Qt::Key_E,              QObject::tr("Open SQL editor"))
    CFG_KEY_ENTRY(OPEN_DDL_HISTORY,       Qt::CTRL | Qt::Key_H,             QObject::tr("Open DDL history window"))
    CFG_KEY_ENTRY(OPEN_SNIPPETS_EDITOR,   Qt::CTRL | Qt::SHIFT | Qt::Key_P, QObject::tr("Open snippets editor window"))
    CFG_KEY_ENTRY(OPEN_FUNCTION_EDITOR,   Qt::CTRL | Qt::SHIFT | Qt::Key_F, QObject::tr("Open function editor window"))
    CFG_KEY_ENTRY(OPEN_COLLATION_EDITOR,  Qt::CTRL | Qt::SHIFT | Qt::Key_C, QObject::tr("Open collation editor window"))
    CFG_KEY_ENTRY(OPEN_EXTENSION_MANAGER, Qt::CTRL | Qt::SHIFT | Qt::Key_E, QObject::tr("Open extension manager window"))
    CFG_KEY_ENTRY(PREV_TASK,              PREV_TASK_KEY_SEQ,                QObject::tr("Previous window"))
    CFG_KEY_ENTRY(NEXT_TASK,              NEXT_TASK_KEY_SEQ,                QObject::tr("Next window"))
    CFG_KEY_ENTRY(HIDE_STATUS_FIELD,      Qt::Key_Escape,                   QObject::tr("Hide status area"))
    CFG_KEY_ENTRY(USER_MANUAL,            Qt::Key_F1,                       QObject::tr("Open user manual"))
    CFG_KEY_ENTRY(OPEN_CONFIG,            Qt::Key_F10,                      QObject::tr("Open configuration dialog"))
    CFG_KEY_ENTRY(OPEN_DEBUG_CONSOLE,     Qt::Key_F12,                      QObject::tr("Open Debug Console"))
    CFG_KEY_ENTRY(OPEN_CSS_CONSOLE,       Qt::Key_F11,                      QObject::tr("Open CSS Console"))
    CFG_KEY_ENTRY(ABOUT,                  Qt::SHIFT | Qt::Key_F1,           QObject::tr("Open the About dialog"))
    CFG_KEY_ENTRY(QUIT,                   Qt::CTRL | Qt::Key_Q,             QObject::tr("Quit the application"))
)

class GUI_API_EXPORT MainWindow : public QMainWindow, public ExtActionContainer
{
    Q_OBJECT

    public:
        enum Action
        {
            MDI_TILE,
            MDI_CASCADE,
            MDI_TILE_HORIZONTAL,
            MDI_TILE_VERTICAL,
            OPEN_SQL_EDITOR,
            NEXT_TASK,
            PREV_TASK,
            HIDE_STATUS_FIELD,
            OPEN_CONFIG,
            OPEN_DDL_HISTORY,
            OPEN_SNIPPETS_EDITOR,
            OPEN_FUNCTION_EDITOR,
            OPEN_COLLATION_EDITOR,
            OPEN_EXTENSION_MANAGER,
            EXPORT,
            IMPORT,
            CLOSE_WINDOW,
            CLOSE_ALL_WINDOWS,
            CLOSE_ALL_WINDOWS_LEFT,
            CLOSE_ALL_WINDOWS_RIGHT,
            CLOSE_OTHER_WINDOWS,
            RESTORE_WINDOW,
            RENAME_WINDOW,
            OPEN_DEBUG_CONSOLE,
            OPEN_CSS_CONSOLE,
            LICENSES,
            HOMEPAGE,
            USER_MANUAL,
            SQLITE_DOCS,
            REPORT_BUG,
            FEATURE_REQUEST,
            ABOUT,
            DONATE,
            BUG_REPORT_HISTORY,
            CHECK_FOR_UPDATES,
            QUIT
        };
        Q_ENUM(Action)

        enum ToolBar
        {
            TOOLBAR_MAIN,
            TOOLBAR_DATABASE,
            TOOLBAR_STRUCTURE,
            TOOLBAR_VIEW
        };

        static MainWindow* getInstance();
        static void setSafeMode(bool enabled);
        static bool isSafeMode();

        MdiArea* getMdiArea() const;
        DbTree* getDbTree() const;
        StatusField* getStatusField() const;
        void restoreSession();
        bool setStyle(const QString& styleName);
        FormManager* getFormManager() const;
        bool eventFilter(QObject* obj, QEvent* e);
        void pushClosedWindowSessionValue(const QVariant& value);
        bool hasClosedWindowToRestore() const;
        bool isClosingApp() const;
        QToolBar* getToolBar(int toolbar) const;
        void openDb(const QString& path);
        QMenu* getDatabaseMenu() const;
        QMenu* getStructureMenu() const;
        QMenu* getViewMenu() const;
        QMenu* getToolsMenu() const;
        QMenu* getSQLiteStudioMenu() const;
        QString currentStyle() const;
        ThemeTuner* getThemeTuner() const;
        EditorWindow* openSqlEditor(Db* dbToSet, const QString& sql);

        static_char* ALLOW_MULTIPLE_SESSIONS_SETTING = "AllowMultipleSessions";

    protected:
        void closeEvent(QCloseEvent *event);

    private:
        MainWindow();
        ~MainWindow();

        void init();
        void observeSessionChanges();
        void createActions();
        void setupDefShortcuts();
        void initMenuBar();
        void saveSession(MdiWindow* currWindow);
        void saveSession(bool hide);
        void restoreWindowSessions(const QList<QVariant>& windowSessions);
        MdiWindow *restoreWindowSession(const QVariant& windowSessions);
        void closeNonSessionWindows();
        DdlHistoryWindow* openDdlHistory();
        FunctionsEditor* openFunctionEditor();
        CodeSnippetEditor* openCodeSnippetEditor();
        CollationsEditor* openCollationEditor();
        SqliteExtensionEditor* openExtensionManager();
        void fixFonts();
        void fixToolbars();

        template <class T>
        T* openMdiWindow();

        static bool confirmQuit(const QList<Committable*>& instances);

        static MainWindow* instance;
        static bool safeModeEnabled;
        static constexpr int closedWindowsStackSize = 20;
        static_char* openUpdatesUrl = "open_updates://";
        static constexpr int saveSessionDelayMs = 500;

        Ui::MainWindow *ui = nullptr;
        DbTree* dbTree = nullptr;
        StatusField* statusField = nullptr;
        QMenu* mdiMenu = nullptr;
        FormManager* formManager = nullptr;
        QQueue<QVariant> closedWindowSessionValues;
        bool closingApp = false;
        QMenu* dbMenu = nullptr;
        QMenu* structMenu = nullptr;
        QMenu* viewMenu = nullptr;
        QMenu* toolsMenu = nullptr;
        QMenu* sqlitestudioMenu = nullptr;
#ifdef PORTABLE_CONFIG
        QPointer<NewVersionDialog> newVersionDialog;
#endif
        WidgetCover* widgetCover = nullptr;
        bool manualUpdatesChecking = false;
        QTimer* saveSessionTimer = nullptr;

    public slots:
        EditorWindow* openSqlEditor();
        void updateWindowActions();
        void updateCornerDocking();
        void messageFromSecondaryInstance(quint32 instanceId, QByteArray message);
        void licenses();
        void homepage();
        void githubReleases();
        void userManual();
        void sqliteDocs();
        void reportHistory();
        void donate();

    private slots:
        void notifyAboutLanguageChange();
        void cleanUp();
        void openSqlEditorSlot();
        void refreshMdiWindows();
        void hideStatusField();
        void openConfig();
        void openDdlHistorySlot();
        void openFunctionEditorSlot();
        void openCodeSnippetsEditorSlot();
        void openCollationEditorSlot();
        void openExtensionManagerSlot();
        void exportAnything();
        void importAnything();
        void closeAllWindows();
        void closeAllLeftWindows();
        void closeAllRightWindows();
        void closeAllWindowsButSelected();
        void closeSelectedWindow();
        void restoreLastClosedWindow();
        void renameWindow();
        void openDebugConsole();
        void openCssConsole();
        void reportBug();
        void requestFeature();
        void aboutSqlitestudio();
#ifdef PORTABLE_CONFIG
        void updateAvailable(const QString& version, const QString& url);
        void noUpdatesAvailable();
        void checkForUpdates();
#endif
        void statusFieldLinkClicked(const QString& link);
        void quit();
        void updateMultipleSessionsSetting();
        void updateMultipleSessionsSetting(const QVariant& newValue);
        void saveSession();
        void scheduleSessionSave();

    signals:
        void sessionValueChanged();
};

template <class T>
T* MainWindow::openMdiWindow()
{
    T* win = nullptr;
    for (MdiWindow* mdiWin : ui->mdiArea->getWindows())
    {
        win = dynamic_cast<T*>(mdiWin->getMdiChild());
        if (win)
        {
            ui->mdiArea->setActiveSubWindow(mdiWin);
            return win;
        }
    }

    win = new T(ui->mdiArea);
    if (win->isInvalid())
    {
        delete win;
        return nullptr;
    }

    ui->mdiArea->addSubWindow(win);
    return win;
}


#define MAINWINDOW MainWindow::getInstance()

#endif // MAINWINDOW_H
