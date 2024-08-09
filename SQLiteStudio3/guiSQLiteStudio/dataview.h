#ifndef DATAVIEW_H
#define DATAVIEW_H

#include "common/extactioncontainer.h"
#include "guiSQLiteStudio_global.h"
#include <QTabWidget>
#include <QMutex>

class QToolBar;
class SqlQueryView;
class SqlQueryModel;
class SqlQueryItem;
class FormView;
class ExtLineEdit;
class QLabel;
class IntValidator;
class WidgetCover;
class QScrollArea;
class QLineEdit;

CFG_KEY_LIST(DataView, QObject::tr("Data view (both grid and form)"),
     CFG_KEY_ENTRY(REFRESH_DATA,    Qt::Key_F5,                   QObject::tr("Refresh data"))
     CFG_KEY_ENTRY(SHOW_GRID_VIEW,  Qt::CTRL | Qt::Key_Comma,     QObject::tr("Switch to grid view of the data"))
     CFG_KEY_ENTRY(SHOW_FORM_VIEW,  Qt::CTRL | Qt::Key_Period,    QObject::tr("Switch to form view of the data"))
)

class GUI_API_EXPORT DataView : public QTabWidget, public ExtActionContainer
{
    Q_OBJECT

    public:
        enum Action
        {
            SHOW_GRID_VIEW,
            SHOW_FORM_VIEW,
            TABS_ON_TOP,
            TABS_AT_BOTTOM,
            // Grid view
            REFRESH_DATA,
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
            FILTER_EXACT,
            FILTER_PER_COLUMN,
            GRID_TOTAL_ROWS,
            SELECTIVE_COMMIT,
            SELECTIVE_ROLLBACK,
            INSERT_ROW_BEFORE,
            INSERT_ROW_AFTER,
            INSERT_ROW_AT_END,
            // Form view
            FORM_TOTAL_ROWS,
            FORM_CURRENT_ROW
        };
        Q_ENUM(Action)

        enum class ActionGroup
        {
            FILTER_MODE,
            TABS_POSITION,
            INSERT_ROW_POSITIONING
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
        bool isUncommitted() const;

        static void staticInit();
        static void insertAction(ExtActionPrototype* action, ToolBar toolbar = TOOLBAR_GRID);
        static void insertActionBefore(ExtActionPrototype* action, Action beforeAction, ToolBar toolbar = TOOLBAR_GRID);
        static void insertActionAfter(ExtActionPrototype* action, Action afterAction, ToolBar toolbar = TOOLBAR_GRID);
        static void removeAction(ExtActionPrototype* action, ToolBar toolbar = TOOLBAR_GRID);

    protected:
        void createActions();
        void setupDefShortcuts();
        void resizeColumnsInitiallyToContents();

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
            REGEXP,
            EXACT
        };

        static void createStaticActions();
        static void loadTabsMode();

        void initFormView();
        void initFilter();
        void initUpdates();
        void initSlots();
        void initPageEdit();
        void initWidgetCover();
        void createContents();
        void createFilterPanel();
        void goToFormRow(IndexModifier idxMod);
        void setNavigationState(bool enabled);
        void updateNavigationState();
        void updateGridNavigationState();
        void goToPage(const QString& pageStr);
        void updatePageEdit();
        void updateResultsCount(int resultsCount);
        void updateCurrentFormViewRow();
        void setFormViewEnabled(bool enabled);
        void readData();
        void initFormViewForNewRow();
        void formViewFocusFirstEditor();
        void recreateFilterInputs();
        void createFilteringActions();
        void setActionIcon(QAction *action, const QIcon &icon, QToolBar *toolbar);

        static TabsPosition tabsPosition;
        static QHash<Action,QAction*> staticActions;
        static QHash<ActionGroup,QActionGroup*> staticActionGroups;

        FilterMode filterMode = FilterMode::STRING;
        QToolBar* gridToolBar = nullptr;
        QToolBar* formToolBar = nullptr;
        SqlQueryView* gridView = nullptr;
        SqlQueryModel* model = nullptr;
        FormView* formView = nullptr;
        QWidget* gridWidget = nullptr;
        QWidget* formWidget = nullptr;
        QScrollArea* perColumnFilterArea = nullptr;
        QWidget* perColumnWidget = nullptr;
        QWidget* perColumnAreaParent = nullptr;
        ExtLineEdit* filterEdit = nullptr;
        QLabel* rowCountLabel = nullptr;
        QLabel* formViewRowCountLabel = nullptr;
        QLabel* formViewCurrentRowLabel = nullptr;
        ExtLineEdit* pageEdit = nullptr;
        IntValidator* pageValidator = nullptr;
        bool navigationState = false;
        bool totalPagesAvailable = false;
        QMutex manualPageChangeMutex;
        bool uncommittedGrid = false;
        bool uncommittedForm = false;
        WidgetCover* widgetCover = nullptr;
        QList<ExtLineEdit*> filterInputs;
        QStringList filterValues;
        QWidget* filterLeftSpacer = nullptr;
        QWidget* filterRightSpacer = nullptr;

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
        void columnsHeaderDoubleClicked(int columnIdx);
        void tabChanged(int newIndex);
        void updateFormNavigationState();
        void updateFormCommitRollbackActions();
        void updateCommitRollbackActions(bool enabled);
        void updateSelectiveCommitRollbackActions(bool enabled);
        void showGridView();
        void showFormView();
        void updateTabsMode();
        void filterModeSelected();
        void coverForGridCommit(int total);
        void updateGridCommitCover(int value);
        void hideGridCommitCover();
        void adjustColumnWidth(SqlQueryItem* item);
        void syncFilterScrollPosition();
        void resizeFilter(int section, int oldSize, int newSize);
        void togglePerColumnFiltering();
};

TYPE_OF_QHASH qHash(DataView::ActionGroup action);

#endif // DATAVIEW_H
