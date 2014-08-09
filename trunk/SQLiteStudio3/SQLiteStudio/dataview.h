#ifndef DATAVIEW_H
#define DATAVIEW_H

#include "common/extactioncontainer.h"
#include <QTabWidget>
#include <QMutex>

class QToolBar;
class SqlQueryView;
class SqlQueryModel;
class FormView;
class ExtLineEdit;
class QLabel;
class IntValidator;

CFG_KEY_LIST(DataView, QObject::tr("Data view (both grid and form)"),
     CFG_KEY_ENTRY(REFRESH_DATA,    Qt::Key_F5,                 QObject::tr("Refresh data"))
//     CFG_KEY_ENTRY(COMMIT_GRID,     Qt::CTRL + Qt::Key_Return) // TODO this was commented - check why and if we can uncomment it
//     CFG_KEY_ENTRY(ROLLBACK_GRID,   Qt::CTRL + Qt::Key_Backspace) // TODO this was commented - check why and if we can uncomment it
     CFG_KEY_ENTRY(DELETE_ROW,      Qt::Key_Delete,             QObject::tr("Delete selected data row"))
     CFG_KEY_ENTRY(INSERT_ROW,      Qt::Key_Insert,             QObject::tr("Insert new data row"))
     CFG_KEY_ENTRY(SHOW_GRID_VIEW,  Qt::CTRL + Qt::Key_Comma,   QObject::tr("Switch to grid view of the data"))
     CFG_KEY_ENTRY(SHOW_FORM_VIEW,  Qt::CTRL + Qt::Key_Period,  QObject::tr("Switch to form view of the data"))
)

class DataView : public QTabWidget, public ExtActionContainer
{
        Q_OBJECT
        Q_ENUMS(Action)

    public:
        enum Action
        {
            SHOW_GRID_VIEW,
            SHOW_FORM_VIEW,
            TABS_ON_TOP,
            TABS_AT_BOTTOM,
            // Grid view
            REFRESH_DATA,
            INSERT_ROW,
            INSERT_MULTIPLE_ROWS,
            DELETE_ROW,
            COMMIT_GRID,
            ROLLBACK_GRID,
            FIRST_PAGE,
            PREV_PAGE,
            NEXT_PAGE,
            LAST_PAGE,
            PAGE_EDIT,
            FILTER_VALUE,
            FILTER,
            FILTER_STRING,
            FILTER_SQL,
            FILTER_REGEXP,
            GRID_TOTAL_ROWS,
            SELECTIVE_COMMIT,
            SELECTIVE_ROLLBACK,
            // Form view
            COMMIT_FORM,
            ROLLBACK_FORM,
            FIRST_ROW,
            PREV_ROW,
            NEXT_ROW,
            LAST_ROW,
            FORM_TOTAL_ROWS
        };

        enum class ActionGroup
        {
            FILTER_MODE,
            TABS_POSITION
        };

        enum ToolBar
        {
            TOOLBAR_GRID,
            TOOLBAR_FORM
        };

        explicit DataView(QWidget *parent = 0);

        void init(SqlQueryModel* model);

        FormView* getFormView() const;
        SqlQueryView* getGridView() const;
        SqlQueryModel* getModel() const;
        QToolBar* getToolBar(int toolbar) const;

        static void staticInit();
        static void insertAction(ExtActionPrototype* action, ToolBar toolbar = TOOLBAR_GRID);
        static void insertActionBefore(ExtActionPrototype* action, Action beforeAction, ToolBar toolbar = TOOLBAR_GRID);
        static void insertActionAfter(ExtActionPrototype* action, Action afterAction, ToolBar toolbar = TOOLBAR_GRID);
        static void removeAction(ExtActionPrototype* action, ToolBar toolbar = TOOLBAR_GRID);

    protected:
        void createActions();
        void setupDefShortcuts();

    private:
        enum class TabsPosition
        {
            TOP,
            BOTTOM
        };

        enum class IndexModifier
        {
            FIRST,
            PREV,
            NEXT,
            LAST
        };

        enum class FilterMode
        {
            STRING,
            SQL,
            REGEXP
        };

        static void createStaticActions();
        static void loadTabsMode();

        void initFormView();
        void initFilter();
        void initUpdates();
        void initSlots();
        void initPageEdit();
        void createContents();
        void goToFormRow(IndexModifier idxMod);
        void setNavigationState(bool enabled);
        void updateNavigationState();
        void updateGridNavigationState();
        void goToPage(const QString& pageStr);
        void updatePageEdit();
        void updateResultsCount(int resultsCount);
        void setFormViewEnabled(bool enabled);
        void readData();
        void updateFilterIcon();

        static FilterMode filterMode;
        static TabsPosition tabsPosition;
        static QHash<Action,QAction*> staticActions;
        static QHash<ActionGroup,QActionGroup*> staticActionGroups;

        QToolBar* gridToolBar;
        QToolBar* formToolBar;
        SqlQueryView* gridView;
        SqlQueryModel* model;
        FormView* formView;
        QWidget* gridWidget;
        QWidget* formWidget;
        ExtLineEdit* filterEdit;
        QLabel* rowCountLabel;
        QLabel* formViewRowCountLabel;
        ExtLineEdit* pageEdit;
        IntValidator* pageValidator;
        bool navigationState = false;
        bool totalPagesAvailable = false;
        QMutex manualPageChangeMutex;

    signals:

    public slots:
        void refreshData();

    private slots:
        void dataLoadingEnded(bool successful);
        void executionSuccessful();
        void totalRowsAndPagesAvailable();
        void insertRow();
        void insertMultipleRows();
        void deleteRow();
        void commitGrid();
        void rollbackGrid();
        void selectiveCommitGrid();
        void selectiveRollbackGrid();
        void firstPage();
        void prevPage();
        void nextPage();
        void lastPage();
        void pageEntered();
        void applyFilter();
        void resetFilter();
        void commitForm();
        void rollbackForm();
        void firstRow();
        void prevRow();
        void nextRow();
        void lastRow();
        void columnsHeaderClicked(int columnIdx);
        void tabChanged(int newIndex);
        void updateFormNavigationState();
        void updateFormCommitRollbackActions();
        void updateCommitRollbackActions(bool enabled);
        void updateSelectiveCommitRollbackActions(bool enabled);
        void showGridView();
        void showFormView();
        void updateTabsMode();
        void filterModeSelected();
};

int qHash(DataView::ActionGroup action);

#endif // DATAVIEW_H
