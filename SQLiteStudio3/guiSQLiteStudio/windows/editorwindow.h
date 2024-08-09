#ifndef EDITOR_H
#define EDITOR_H

#include "db/db.h"
#include "mdichild.h"
#include "common/extactioncontainer.h"
#include "guiSQLiteStudio_global.h"
#include <QWidget>

namespace Ui {
    class EditorWindow;
}

class SqlQueryModel;
class QComboBox;
class QActionGroup;
class DbListModel;
class QLabel;
class QLineEdit;
class ExtLineEdit;
class IntValidator;
class FormView;
class SqlQueryItem;
class SqlEditor;
class DbComboBox;

CFG_KEY_LIST(EditorWindow, QObject::tr("SQL editor window"),
     CFG_KEY_ENTRY(EXEC_QUERY,                Qt::Key_F9,                 QObject::tr("Execute query"))
     CFG_KEY_ENTRY(EXEC_ONE_QUERY,            Qt::CTRL | Qt::Key_F9,      QObject::tr("Execute single query under cursor"))
     CFG_KEY_ENTRY(EXEC_ALL_QUERIES,          Qt::SHIFT | Qt::Key_F9,     QObject::tr("Execute all queries in editor"))
     CFG_KEY_ENTRY(EXPLAIN_QUERY,             Qt::Key_F8,                 QObject::tr("Execute \"%1\" query").arg("EXPLAIN"))
     CFG_KEY_ENTRY(PREV_DB,                   Qt::CTRL | Qt::Key_Up,      QObject::tr("Switch current working database to previous on the list"))
     CFG_KEY_ENTRY(NEXT_DB,                   Qt::CTRL | Qt::Key_Down,    QObject::tr("Switch current working database to next on the list"))
     CFG_KEY_ENTRY(SHOW_NEXT_TAB,             Qt::ALT | Qt::Key_Right,    QObject::tr("Go to next editor tab"))
     CFG_KEY_ENTRY(SHOW_PREV_TAB,             Qt::ALT | Qt::Key_Left,     QObject::tr("Go to previous editor tab"))
     CFG_KEY_ENTRY(FOCUS_RESULTS_BELOW,       Qt::ALT | Qt::Key_PageDown, QObject::tr("Move keyboard input focus to the results view below"))
     CFG_KEY_ENTRY(FOCUS_EDITOR_ABOVE,        Qt::ALT | Qt::Key_PageUp,   QObject::tr("Move keyboard input focus to the SQL editor above"))
     CFG_KEY_ENTRY(DELETE_SINGLE_HISTORY_SQL, QKeySequence::Delete,       QObject::tr("Delete selected SQL history entries"))
)

class GUI_API_EXPORT EditorWindow : public MdiChild
{
    Q_OBJECT

    public:
        enum class ResultsDisplayMode
        {
            SEPARATE_TAB = 0,
            BELOW_QUERY = 1
        };

        enum Action
        {
            EXEC_QUERY,
            EXEC_ONE_QUERY,
            EXEC_ALL_QUERIES,
            EXPLAIN_QUERY,
            RESULTS_IN_TAB,
            RESULTS_BELOW,
            CURRENT_DB,
            NEXT_DB,
            PREV_DB,
            SHOW_NEXT_TAB,
            SHOW_PREV_TAB,
            FOCUS_RESULTS_BELOW,
            FOCUS_EDITOR_ABOVE,
            CLEAR_HISTORY,
            EXPORT_RESULTS,
            CREATE_VIEW_FROM_QUERY,
            DELETE_SINGLE_HISTORY_SQL
        };
        Q_ENUM(Action)

        enum QueryExecMode
        {
            DEFAULT,
            SINGLE,
            ALL
        };

        enum ToolBar
        {
            TOOLBAR_MAIN
        };

        enum class ActionGroup
        {
            RESULTS_POSITIONING
        };

        explicit EditorWindow(QWidget *parent = 0);
        EditorWindow(const EditorWindow& editor);
        ~EditorWindow();

        static void staticInit();
        static void insertAction(ExtActionPrototype* action, ToolBar toolbar = TOOLBAR_MAIN);
        static void insertActionBefore(ExtActionPrototype* action, Action beforeAction, ToolBar toolbar = TOOLBAR_MAIN);
        static void insertActionAfter(ExtActionPrototype* action, Action afterAction, ToolBar toolbar = TOOLBAR_MAIN);
        static void removeAction(ExtActionPrototype* action, ToolBar toolbar = TOOLBAR_MAIN);

        QSize sizeHint() const;
        QAction* getAction(Action action);
        QString getQueryToExecute(bool doSelectCurrentQuery = false, QueryExecMode querySelectionMode = DEFAULT);
        bool setCurrentDb(Db* db);
        void setContents(const QString& sql);
        QString getContents() const;
        void execute();
        QToolBar* getToolBar(int toolbar) const;
        SqlEditor* getEditor() const;
        bool isUncommitted() const;
        QString getQuitUncommittedConfirmMessage() const;
        Db* getCurrentDb();

    protected:
        void changeEvent(QEvent *e);
        QVariant saveSession();
        bool restoreSession(const QVariant& sessionValue);
        Icon* getIconNameForMdiWindow();
        QString getTitleForMdiWindow();

    private:
        static void createStaticActions();
        static void loadTabsMode();

        void init();
        void createActions();
        void createDbCombo();
        void setupDefShortcuts();
        void selectCurrentQuery(bool fallBackToPreviousIfNecessary = false);
        void updateShortcutTips();
        void setupSqlHistoryMenu();
        bool processBindParams(QString& sql, QHash<QString, QVariant>& queryParams);

        static const int queryLimitForSmartExecution = 100;

        static ResultsDisplayMode resultsDisplayMode;
        static QHash<Action,QAction*> staticActions;
        static QHash<ActionGroup,QActionGroup*> staticActionGroups;

        Ui::EditorWindow *ui = nullptr;
        SqlQueryModel* resultsModel = nullptr;
        QHash<ActionGroup,QActionGroup*> actionGroups;
        DbComboBox* dbCombo = nullptr;
        int sqlEditorNum = 1;
        qint64 lastQueryHistoryId = 0;
        QString lastSuccessfulQuery;
        QMenu* sqlHistoryMenu = nullptr;
        bool settingSqlContents = false;

    private slots:
        void execQuery(bool explain = false, QueryExecMode querySelectionMode = DEFAULT);
        void execOneQuery();
        void execAllQueries();
        void explainQuery();
        void dbChanged();
        void executionSuccessful();
        void executionFailed(const QString& errorText);
        void storeExecutionInHistory();
        void updateResultsDisplayMode();
        void prevDb();
        void nextDb();
        void showNextTab();
        void showPrevTab();
        void focusResultsBelow();
        void focusEditorAbove();
        void historyEntrySelected(const QModelIndex& current, const QModelIndex& previous);
        void historyEntryActivated(const QModelIndex& current);
        void deleteSelectedSqlHistory();
        void clearHistory();
        void sqlHistoryContextMenuRequested(const QPoint &pos);
        void exportResults();
        void createViewFromQuery();
        void updateState();
        void checkTextChangedForSession();
        void queryHighlightingConfigChanged(const QVariant& enabled);

    public slots:
        void refreshValidDbObjects();
};

GUI_API_EXPORT TYPE_OF_QHASH qHash(EditorWindow::ActionGroup action);

#endif // EDITOR_H
