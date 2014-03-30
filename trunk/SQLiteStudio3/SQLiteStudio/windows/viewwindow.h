#ifndef VIEWWINDOW_H
#define VIEWWINDOW_H

#include "mdichild.h"
#include "common/extactioncontainer.h"
#include "db/db.h"
#include "parser/ast/sqlitecreateview.h"
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

class ViewWindow : public MdiChild, public ExtActionContainer
{
        Q_OBJECT

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

    public:
        explicit ViewWindow(QWidget *parent = 0);
        ViewWindow(Db* db, QWidget *parent = 0);
        ViewWindow(const ViewWindow& win);
        ViewWindow(QWidget *parent, Db* db, const QString& database, const QString& view);
        ~ViewWindow();

        Db* getDb() const;
        QString getDatabase() const;
        QString getView() const;

        static void staticInit();

    protected:
        void changeEvent(QEvent *e);
        QVariant saveSession();
        bool restoreSession(const QVariant& sessionValue);
        Icon* getIconNameForMdiWindow();
        QString getTitleForMdiWindow();
        void createActions();
        void setupDefShortcuts();
        bool restoreSessionNextTime();

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
        QPushButton* coverCancelButton;
        QProgressBar* coverBusyBar;
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
        void disableCoverCancelButton();
        void changesSuccessfullyCommited();
        void changesFailedToCommit(int errorCode, const QString& errorText);
        void updateTriggersState();
        void nextTab();
        void prevTab();

    public slots:
        void refreshTriggers();
};

#endif // VIEWWINDOW_H
