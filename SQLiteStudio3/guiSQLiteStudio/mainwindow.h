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
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
#include <QtSystemDetection>
#else
#include <qsystemdetection.h>
#endif
#include <QMimeDatabase>

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
class CodeSnippetsPanel;

#ifdef Q_OS_MACX
#define PREV_TASK_KEY_SEQ         Qt::CTRL | Qt::ALT | Qt::Key_Left
#define NEXT_TASK_KEY_SEQ         Qt::CTRL | Qt::ALT | Qt::Key_Right
#define MOVE_TASK_EARLIER_KEY_SEQ Qt::CTRL | Qt::SHIFT | Qt::ALT | Qt::Key_Left
#define MOVE_TASK_LATER_KEY_SEQ   Qt::CTRL | Qt::SHIFT | Qt::ALT | Qt::Key_Right
#else
#define PREV_TASK_KEY_SEQ         Qt::CTRL | Qt::Key_PageUp
#define NEXT_TASK_KEY_SEQ         Qt::CTRL | Qt::Key_PageDown
#define MOVE_TASK_EARLIER_KEY_SEQ Qt::CTRL | Qt::SHIFT | Qt::Key_PageUp
#define MOVE_TASK_LATER_KEY_SEQ   Qt::CTRL | Qt::SHIFT | Qt::Key_PageDown
#endif


class MouseShortcut;
CFG_KEY_LIST(MainWindow, QObject::tr("Main window"),
    CFG_KEY_ENTRY(OPEN_SQL_EDITOR,        Qt::CTRL | Qt::Key_T,             QObject::tr("Open SQL editor"))
    CFG_KEY_ENTRY(RESTORE_WINDOW,         Qt::CTRL | Qt::SHIFT | Qt::Key_T, QObject::tr("Restore recently closed window"))
    CFG_KEY_ENTRY(EXPORT,                 Qt::CTRL | Qt::SHIFT | Qt::Key_E, QObject::tr("Open Export Dialog"))
    CFG_KEY_ENTRY(IMPORT,                 Qt::CTRL | Qt::SHIFT | Qt::Key_I, QObject::tr("Open Import Dialog"))
    CFG_KEY_ENTRY(OPEN_DDL_HISTORY,       Qt::CTRL | Qt::Key_H,             QObject::tr("Open DDL history window"))
    CFG_KEY_ENTRY(OPEN_FUNCTION_EDITOR,   Qt::ALT | Qt::Key_1,              QObject::tr("Open function editor window"))
    CFG_KEY_ENTRY(OPEN_SNIPPETS_EDITOR,   Qt::ALT | Qt::Key_2,              QObject::tr("Open snippets editor window"))
    CFG_KEY_ENTRY(OPEN_COLLATION_EDITOR,  Qt::ALT | Qt::Key_3,              QObject::tr("Open collation editor window"))
    CFG_KEY_ENTRY(OPEN_EXTENSION_MANAGER, Qt::ALT | Qt::Key_4,              QObject::tr("Open extension manager window"))
    CFG_KEY_ENTRY(PREV_TASK,              PREV_TASK_KEY_SEQ,                QObject::tr("Previous window"))
    CFG_KEY_ENTRY(NEXT_TASK,              NEXT_TASK_KEY_SEQ,                QObject::tr("Next window"))
    CFG_KEY_ENTRY(MOVE_TASK_EARLIER,      MOVE_TASK_EARLIER_KEY_SEQ,        QObject::tr("Move window earlier"))
    CFG_KEY_ENTRY(MOVE_TASK_LATER,        MOVE_TASK_LATER_KEY_SEQ,          QObject::tr("Move window later"))
    CFG_KEY_ENTRY(HIDE_STATUS_FIELD,      Qt::Key_Escape,                   QObject::tr("Hide status area"))
    CFG_KEY_ENTRY(USER_MANUAL,            Qt::Key_F1,                       QObject::tr("Open user manual"))
    CFG_KEY_ENTRY(OPEN_CONFIG,            Qt::CTRL | Qt::Key_Comma,         QObject::tr("Open configuration dialog"))
    CFG_KEY_ENTRY(OPEN_DEBUG_CONSOLE,     Qt::Key_F12,                      QObject::tr("Open Debug Console"))
    CFG_KEY_ENTRY(OPEN_CSS_CONSOLE,       Qt::Key_F11,                      QObject::tr("Open CSS Console"))
    CFG_KEY_ENTRY(ABOUT,                  Qt::SHIFT | Qt::Key_F1,           QObject::tr("Open the About dialog"))
    CFG_KEY_ENTRY(QUIT,                   QKeySequence::Quit,               QObject::tr("Quit the application"))
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
            TOOLBAR_ICON_SIZE_50,
            TOOLBAR_ICON_SIZE_75,
            TOOLBAR_ICON_SIZE_100,
            TOOLBAR_ICON_SIZE_125,
            TOOLBAR_ICON_SIZE_150,
            TOOLBAR_ICON_SIZE_175,
            TOOLBAR_ICON_SIZE_200,
            TOOLBAR_ICON_SIZE_250,
            TOOLBAR_ICON_SIZE_300,
            OPEN_SQL_EDITOR,
            NEXT_TASK,
            PREV_TASK,
            MOVE_TASK_LATER,
            MOVE_TASK_EARLIER,
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
            QUIT,
            NEW_DB,
            OPEN_FILE,
            EXPORT_SETTINGS,
            IMPORT_SETTINGS,
        };
        Q_ENUM(Action)

        enum ToolBar
        {
            TOOLBAR_MAIN,
            TOOLBAR_DATABASE,
            TOOLBAR_STRUCTURE,
            TOOLBAR_VIEW
        };

        enum class DropFileType
        {
            SQLITE3,
            SQLITE3_POSSIBLE,
            SQLITE3_EMPTY,
            SQL,
            TEXT,
            CSV,
            SQLITE2,
            OTHER
        };

        struct DropFileContext
        {
            QString mimeValue;
            DropFileType type;
            QString fileName;
            QString fullPath;
        };

        static MainWindow* getInstance();
        static void setSafeMode(bool enabled);
        static bool isSafeMode();
        static bool isSessionRestoringFinished();
        static bool isInternalDrop(const QMimeData *data);
        static DropFileType mimeToFileType(const QString& mimeValue);
        static DropFileType fileToFileType(const QString& filePath);
        static DropFileContext fileToDropContext(const QString& filePath);

        MdiArea* getMdiArea() const;
        DbTree* getDbTree() const;
        StatusField* getStatusField() const;
        void restoreSession();
        bool setStyle(const QString& styleName);
        FormManager* getFormManager() const;
        bool eventFilter(QObject* obj, QEvent* e) override;
        void pushClosedWindowSessionValue(const QVariant& value);
        bool hasClosedWindowToRestore() const;
        bool isClosingApp() const;
        QToolBar* getToolBar(int toolbar) const override;
        void openDb(const QString& path);
        QMenu* getDatabaseMenu() const;
        QMenu* getStructureMenu() const;
        QMenu* getViewMenu() const;
        QMenu* getToolsMenu() const;
        QMenu* getSQLiteStudioMenu() const;
        QString currentStyle() const;
        ThemeTuner* getThemeTuner() const;
        EditorWindow* openSqlEditor(Db* dbToSet, const QString& sql);
        EditorWindow* openSqlEditorForFile(Db* dbToSet, const QString& fileName);
        DdlHistoryWindow* openDdlHistory();
        FunctionsEditor* openFunctionEditor();
        CodeSnippetEditor* openCodeSnippetEditor();
        CollationsEditor* openCollationEditor();
        SqliteExtensionEditor* openExtensionManager();
        void installToolbarSizeWheelHandler(QToolBar* toolbar);
        QMenu* createPopupMenu() override;

        template <class T, typename... Args>
        T* openMdiWindow(Args&&... args);

        static_char* ALLOW_MULTIPLE_SESSIONS_SETTING = "AllowMultipleSessions";
        static_char* CONSTANT_ICON_SIZE = "ConstantIconSize";
        static int defaultToolbarIconSize;

    protected:
        void closeEvent(QCloseEvent *event) override;

    private:
        class ToolBarStyleEnforcer : public QObject
        {
            public:
                ToolBarStyleEnforcer(QObject* parent = nullptr);

            protected:
                bool eventFilter(QObject* obj, QEvent* event) override;
        };

        MainWindow();
        ~MainWindow();

        void init();
        void observeSessionChanges();
        void createActions() override;
        void setupDefShortcuts() override;
        void initMenuBar();
        void saveSession(MdiWindow* currWindow);
        void saveSession(bool hide);
        void restoreWindowSessions(const QList<QVariant>& windowSessions);
        MdiWindow *restoreWindowSession(const QVariant& windowSessions);
        void closeNonSessionWindows();
        void fixFonts();
        void fixToolbars();
        QMenu* createToolbarStyleMenu(QMenu* parentMenu);
        void applyToolbarStyle(QToolBar* tb);
        void applyToolbarStyle(QList<QToolBar*> tbList);
        void updateToolbarStyleActionState();
        void initToolbarSizeActionList();
        void handlePostRestoreConfigUpdates();
        void initDropOverlay();
        void handleExternalDragEnter(const QStringList& filePaths);
        void handleExternalDragLeave();
        void handleDroppedFile(const QString& filePath);
        QString dropDescriptionByFileType(const DropFileContext& ctx);

        static bool confirmQuit(const QList<Committable*>& instances);

        static MainWindow* instance;
        static bool safeModeEnabled;
        static bool sessionRestoringFinished;
        static constexpr int closedWindowsStackSize = 20;
        static_char* openUpdatesUrl = "open_updates://";
        static constexpr int saveSessionDelayMs = 500;

        Ui::MainWindow *ui = nullptr;
        DbTree* dbTree = nullptr;
        CodeSnippetsPanel* codeSnippetsPanel = nullptr;
        StatusField* statusField = nullptr;
        QMenu* mdiMenu = nullptr;
        FormManager* formManager = nullptr;
        QQueue<QVariant> closedWindowSessionValues;
        bool closingApp = false;
        QMenu* dbMenu = nullptr;
        QMenu* structMenu = nullptr;
        QMenu* viewMenu = nullptr;
        QMenu* tbStyleMenu = nullptr;
        QMenu* toolsMenu = nullptr;
        QMenu* sqlitestudioMenu = nullptr;
#ifdef HAS_UPDATEMANAGER
        QPointer<NewVersionDialog> newVersionDialog;
#endif
        WidgetCover* dropOverlay = nullptr;
        QLabel* dropDetails = nullptr;
        static QMimeDatabase mimeDb;

        QTimer* saveSessionTimer = nullptr;

        QList<Action> toolbarSizeActionList;
        QHash<Action, int> toolbarSizes;
        QHash<int, Action> toolbarSizesReversed;
        MouseShortcut* toolbarSizeWheelHandler = nullptr;

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
        void updateToolbarStyle();

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
#ifdef HAS_UPDATEMANAGER
        void updateAvailable(const QString& version, const QString& url);
        void noUpdatesAvailable(bool enforced);
        void checkForUpdates();
#endif
        void statusFieldLinkClicked(const QString& link);
        void quit();
        void updateMultipleSessionsSetting();
        void updateMultipleSessionsSetting(const QVariant& newValue);
        void saveSession();
        void scheduleSessionSave();
        void toolbarSizeChangeRequested(int steps);
        void refreshSyntaxColors();
        void exportConfig();
        void importConfig();

    signals:
        void sessionValueChanged();
};

template <class T, typename... Args>
T* MainWindow::openMdiWindow(Args&&... args)
{
    T* win = nullptr;
    for (MdiWindow*& mdiWin : ui->mdiArea->getWindows())
    {
        win = dynamic_cast<T*>(mdiWin->getMdiChild());
        if (win && win->shouldReuseForArgs(sizeof...(args), args...))
        {
            ui->mdiArea->setActiveSubWindow(mdiWin);
            return win;
        }
    }

    win = new T(ui->mdiArea, args...);
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
