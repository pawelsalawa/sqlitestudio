#ifndef VIEWWINDOW_H
#define VIEWWINDOW_H

#include "mdichild.h"
#include "common/extactioncontainer.h"
#include "db/db.h"
#include "parser/ast/sqlitecreateview.h"
#include "guiSQLiteStudio_global.h"
#include <QWidget>

namespace Ui {
    class ViewWindow;
}

class SqliteSyntaxHighlighter;
class WidgetCover;
class QPushButton;
class QProgressBar;
class ChainExecutor;
class ViewModifier;
class SqlViewModel;

CFG_KEY_LIST(ViewWindow, QObject::tr("A view window"),
     CFG_KEY_ENTRY(COMMIT_QUERY,     QKeySequence::Save,           QObject::tr("Commit the view's query"))
     CFG_KEY_ENTRY(ROLLBACK_QUERY,   QKeySequence::Cancel,         QObject::tr("Rollback pending changes in the view's query"))
     CFG_KEY_ENTRY(REFRESH_TRIGGERS, Qt::Key_F5,                   QObject::tr("Refresh view trigger list"))
     CFG_KEY_ENTRY(EXECUTE_QUERY,    Qt::Key_F9,                   QObject::tr("Execute the view's query"))
     CFG_KEY_ENTRY(ADD_TRIGGER,      Qt::Key_Insert,               QObject::tr("Add new trigger"))
     CFG_KEY_ENTRY(EDIT_TRIGGER,     Qt::Key_Return,               QObject::tr("Edit selected trigger"))
     CFG_KEY_ENTRY(DEL_TRIGGER,      Qt::Key_Delete,               QObject::tr("Delete selected trigger"))
     CFG_KEY_ENTRY(NEXT_TAB,         Qt::ALT | Qt::Key_Right,      QObject::tr("Go to next tab"))
     CFG_KEY_ENTRY(PREV_TAB,         Qt::ALT | Qt::Key_Left,       QObject::tr("Go to previous tab"))
)

class GUI_API_EXPORT ViewWindow : public MdiChild
{
    Q_OBJECT

    public:
        enum Action
        {
            // Structure tab
            REFRESH_QUERY,
            COMMIT_QUERY,
            ROLLBACK_QUERY,
            ADD_COLUMN,
            EDIT_COLUMN,
            DEL_COLUMN,
            MOVE_COLUMN_UP,
            MOVE_COLUMN_DOWN,
            GENERATE_OUTPUT_COLUMNS,
            // Triggers tab
            REFRESH_TRIGGERS,
            ADD_TRIGGER,
            EDIT_TRIGGER,
            DEL_TRIGGER,
            // All tabs
            NEXT_TAB,
            PREV_TAB,
            EXECUTE_QUERY
        };
        Q_ENUM(Action)

        enum ToolBar
        {
            TOOLBAR_QUERY,
            TOOLBAR_TRIGGERS
        };

        explicit ViewWindow(QWidget *parent = 0);
        ViewWindow(Db* db, QWidget *parent = 0);
        ViewWindow(const ViewWindow& win);
        ViewWindow(QWidget *parent, Db* db, const QString& database, const QString& view);
        ~ViewWindow();

        Db* getDb() const;
        QString getDatabase() const;
        QString getView() const;
        void setSelect(const QString& selectSql);
        bool isUncommitted() const;
        QString getQuitUncommittedConfirmMessage() const;
        Db* getAssociatedDb() const;

        static void staticInit();
        static void insertAction(ExtActionPrototype* action, ToolBar toolbar = TOOLBAR_QUERY);
        static void insertActionBefore(ExtActionPrototype* action, Action beforeAction, ToolBar toolbar = TOOLBAR_QUERY);
        static void insertActionAfter(ExtActionPrototype* action, Action afterAction, ToolBar toolbar = TOOLBAR_QUERY);
        static void removeAction(ExtActionPrototype* action, ToolBar toolbar = TOOLBAR_QUERY);

    protected:
        void changeEvent(QEvent *e);
        QVariant saveSession();
        bool restoreSession(const QVariant& sessionValue);
        Icon* getIconNameForMdiWindow();
        QString getTitleForMdiWindow();
        void createActions();
        void setupDefShortcuts();
        bool restoreSessionNextTime();
        QToolBar* getToolBar(int toolbar) const;

    private:
        void init();
        void updateAfterInit();
        void createDbCombo();
        void newView();
        void initView();
        void setupCoverWidget();
        void createQueryTabActions();
        void createTriggersTabActions();
        void parseDdl();
        void updateDdlTab();
        bool isModified() const;
        bool validate(bool skipWarnings = false);
        void executeStructureChanges();
        QString getCurrentTrigger() const;
        void applyInitialTab();
        QString getCurrentDdl() const;
        QStringList indexedColumnsToNamesOnly(const QList<SqliteIndexedColumn*>& columns) const;
        QStringList collectColumnNames() const;
        void columnsFromViewToList();
        int getDataTabIdx() const;
        int getQueryTabIdx() const;
        int getDdlTabIdx() const;
        void switchToDataAndReload();

        Db* db = nullptr;
        QString database;
        QString view;
        bool existingView = true;
        bool dataLoaded = false;
        int newViewWindowNum = 1;
        bool modified = false;
        SqliteCreateViewPtr originalCreateView;
        SqliteCreateViewPtr createView;
        SqlViewModel* dataModel = nullptr;
        QString originalQuery;
        WidgetCover* widgetCover = nullptr;
        ChainExecutor* structureExecutor = nullptr;
        ViewModifier* viewModifier = nullptr;
        Ui::ViewWindow *ui = nullptr;
        bool modifyingThisView = false;
        QAction* outputColumnsCheck = nullptr;
        QAction* outputColumnsSeparator = nullptr;
        bool tabsMoving = false;
        bool loadDataAfterNextCommit = false;

    private slots:
        void refreshView();
        void executeQuery();
        void commitView(bool skipWarnings = false, bool loadDataAfterNextCommit = false);
        void rollbackView();
        void addTrigger();
        void editTrigger();
        void deleteTrigger();
        void executionSuccessful();
        void executionFailed(const QString& errorMessage);
        void tabChanged(int tabIdx);
        void updateQueryToolbarStatus();
        void changesSuccessfullyCommitted();
        void changesFailedToCommit(int errorCode, const QString& errorText);
        void updateTriggersState();
        void nextTab();
        void prevTab();
        void dbClosedFinalCleanup();
        void checkIfViewDeleted(const QString& database, const QString& object, DbObjectType type);
        void updateOutputColumnsVisibility();
        void addColumn();
        void editColumn();
        void delColumn();
        void moveColumnUp();
        void moveColumnDown();
        void updateColumnButtons();
        void generateOutputColumns();
        void updateTabsOrder();
        void triggerViewDoubleClicked(const QModelIndex& idx);
        void updateFont();
        void dbChanged();
        void handleObjectModified(Db* db, const QString& database, const QString& object);

    public slots:
        void refreshTriggers();
};

#endif // VIEWWINDOW_H
