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
class SqlQueryModel;
class WidgetCover;
class QPushButton;
class QProgressBar;
class ChainExecutor;
class ViewModifier;

CFG_KEY_LIST(ViewWindow, QObject::tr("A view window"),
     CFG_KEY_ENTRY(REFRESH_TRIGGERS, Qt::Key_F5,                QObject::tr("Refresh view trigger list"))
     CFG_KEY_ENTRY(ADD_TRIGGER,      Qt::Key_Insert,            QObject::tr("Add new trigger"))
     CFG_KEY_ENTRY(EDIT_TRIGGER,     Qt::Key_Return,            QObject::tr("Edit selected trigger"))
     CFG_KEY_ENTRY(DEL_TRIGGER,      Qt::Key_Delete,            QObject::tr("Delete selected trigger"))
     CFG_KEY_ENTRY(NEXT_TAB,         Qt::ALT + Qt::Key_Right,   QObject::tr("Go to next tab"))
     CFG_KEY_ENTRY(PREV_TAB,         Qt::ALT + Qt::Key_Left,    QObject::tr("Go to previous tab"))
)

class GUI_API_EXPORT ViewWindow : public MdiChild
{
        Q_OBJECT
        Q_ENUMS(Action)

    public:
        enum Action
        {
            // Structure tab
            REFRESH_QUERY,
            COMMIT_QUERY,
            ROLLBACK_QUERY,
            // Triggers tab
            REFRESH_TRIGGERS,
            ADD_TRIGGER,
            EDIT_TRIGGER,
            DEL_TRIGGER,
            // All tabs
            NEXT_TAB,
            PREV_TAB
        };

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
        void newView();
        void initView();
        void setupCoverWidget();
        void createQueryTabActions();
        void createTriggersTabActions();
        void parseDdl();
        void updateDdlTab();
        bool isModified() const;
        bool validate();
        void executeStructureChanges();
        QString getCurrentTrigger() const;
        void applyInitialTab();

        Db* db = nullptr;
        QString database;
        QString view;
        bool existingView = true;
        bool dataLoaded = false;
        int newViewWindowNum = 1;
        bool modified = false;
        bool blankNameWarningDisplayed = false;
        SqliteCreateViewPtr originalCreateView;
        SqliteCreateViewPtr createView;
        SqlQueryModel* dataModel = nullptr;
        QString originalQuery;
        WidgetCover* widgetCover;
        ChainExecutor* structureExecutor = nullptr;
        ViewModifier* viewModifier = nullptr;
        Ui::ViewWindow *ui;

    private slots:
        void refreshView();
        void commitView();
        void rollbackView();
        void addTrigger();
        void editTrigger();
        void deleteTrigger();
        void executionSuccessful();
        void executionFailed(const QString& errorMessage);
        void tabChanged(int tabIdx);
        void updateQueryToolbarStatus();
        void changesSuccessfullyCommited();
        void changesFailedToCommit(int errorCode, const QString& errorText);
        void updateTriggersState();
        void nextTab();
        void prevTab();
        void dbClosed();
        void checkIfTableDeleted(const QString& database, const QString& object, DbObjectType type);

    public slots:
        void refreshTriggers();
};

#endif // VIEWWINDOW_H
