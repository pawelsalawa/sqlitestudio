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

CFG_KEY_LIST(EditorWindow, QObject::tr("SQL editor window"),
     CFG_KEY_ENTRY(EXEC_QUERY,          Qt::Key_F9,                 QObject::tr("Execute query"))
     CFG_KEY_ENTRY(EXPLAIN_QUERY,       Qt::Key_F8,                 QObject::tr("Execute \"%1\" query").arg("EXPLAIN"))
     CFG_KEY_ENTRY(PREV_DB,             Qt::CTRL + Qt::Key_Up,      QObject::tr("Switch current working database to previous on the list"))
     CFG_KEY_ENTRY(NEXT_DB,             Qt::CTRL + Qt::Key_Down,    QObject::tr("Switch current working database to next on the list"))
     CFG_KEY_ENTRY(SHOW_NEXT_TAB,       Qt::ALT + Qt::Key_Right,    QObject::tr("Go to next editor tab"))
     CFG_KEY_ENTRY(SHOW_PREV_TAB,       Qt::ALT + Qt::Key_Left,     QObject::tr("Go to previous editor tab"))
     CFG_KEY_ENTRY(FOCUS_RESULTS_BELOW, Qt::ALT + Qt::Key_PageDown, QObject::tr("Move keyboard input focus to the results view below"))
     CFG_KEY_ENTRY(FOCUS_EDITOR_ABOVE,  Qt::ALT + Qt::Key_PageUp,   QObject::tr("Move keyboard input focus to the SQL editor above"))
)

class GUI_API_EXPORT EditorWindow : public MdiChild
{
        Q_OBJECT
        Q_ENUMS(Action)

    public:
        enum class ResultsDisplayMode
        {
            SEPARATE_TAB = 0,
            BELOW_QUERY = 1
        };

        enum Action
        {
            EXEC_QUERY,
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
            CREATE_VIEW_FROM_QUERY
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
        QString getQueryToExecute(bool doSelectCurrentQuery = false);
        bool setCurrentDb(Db* db);
        void setContents(const QString& sql);
        QString getContents() const;
        void execute();
        QToolBar* getToolBar(int toolbar) const;
        SqlEditor* getEditor() const;
        bool isUncommited() const;
        QString getQuitUncommitedConfirmMessage() const;

    protected:
        void changeEvent(QEvent *e);
        QVariant saveSession();
        bool restoreSession(const QVariant& sessionValue);
        Icon* getIconNameForMdiWindow();
        QString getTitleForMdiWindow();
        Db* getCurrentDb();

    private:
        static void createStaticActions();
        static void loadTabsMode();

        void init();
        void createActions();
        void createDbCombo();
        void setupDefShortcuts();
        void selectCurrentQuery(bool fallBackToPreviousIfNecessary = false);
        void updateShortcutTips();

        static ResultsDisplayMode resultsDisplayMode;
        static QHash<Action,QAction*> staticActions;
        static QHash<ActionGroup,QActionGroup*> staticActionGroups;

        Ui::EditorWindow *ui = nullptr;
        SqlQueryModel* resultsModel = nullptr;
        QHash<ActionGroup,QActionGroup*> actionGroups;
        QComboBox* dbCombo = nullptr;
        DbListModel* dbComboModel = nullptr;
        int sqlEditorNum = 1;
        qint64 lastQueryHistoryId = 0;
        QString lastSuccessfulQuery;

    private slots:
        void execQuery(bool explain = false);
        void explainQuery();
        void dbChanged();
        void executionSuccessful();
        void executionFailed(const QString& errorText);
        void totalRowsAndPagesAvailable();
        void updateResultsDisplayMode();
        void prevDb();
        void nextDb();
        void showNextTab();
        void showPrevTab();
        void focusResultsBelow();
        void focusEditorAbove();
        void historyEntrySelected(const QModelIndex& current, const QModelIndex& previous);
        void historyEntryActivated(const QModelIndex& current);
        void clearHistory();
        void exportResults();
        void createViewFromQuery();
        void updateState();
};

GUI_API_EXPORT int qHash(EditorWindow::ActionGroup action);

#endif // EDITOR_H
