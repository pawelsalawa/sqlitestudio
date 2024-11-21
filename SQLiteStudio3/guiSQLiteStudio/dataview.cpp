#include "dataview.h"
#include "datagrid/sqlquerymodel.h"
#include "datagrid/sqlqueryview.h"
#include "formview.h"
#include "common/extlineedit.h"
#include "mainwindow.h"
#include "common/intvalidator.h"
#include "common/extaction.h"
#include "iconmanager.h"
#include "themetuner.h"
#include "uiconfig.h"
#include "datagrid/sqlqueryitem.h"
#include "common/widgetcover.h"
#include "common/unused.h"
#include <QDebug>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QToolBar>
#include <QLabel>
#include <QAction>
#include <QTime>
#include <QStyleFactory>
#include <QLineEdit>
#include <QSizePolicy>
#include <QScrollBar>
#include <QToolButton>

CFG_KEYS_DEFINE(DataView)
DataView::TabsPosition DataView::tabsPosition;
QHash<DataView::Action,QAction*> DataView::staticActions;
QHash<DataView::ActionGroup,QActionGroup*> DataView::staticActionGroups;
static const char* DATA_VIEW_FILTER_PROP = "filter";

DataView::DataView(QWidget *parent) :
    QTabWidget(parent)
{
}

void DataView::init(SqlQueryModel* model)
{
    createContents();

    this->model = model;
    this->model->setView(gridView);

    rowCountLabel = new QLabel();
    formViewRowCountLabel = new QLabel();
    formViewCurrentRowLabel = new QLabel();

    initWidgetCover();
    initFormView();
    initPageEdit();
    initFilter();
    initActions();
    initUpdates();
    initSlots();
    updateTabsMode();
}

void DataView::initUpdates()
{
    updatePageEdit();
    updateFormNavigationState();
    updateGridNavigationState();
    updateCommitRollbackActions(false);
}

void DataView::initSlots()
{
    connect(model, SIGNAL(executionSuccessful()), this, SLOT(executionSuccessful()));
    connect(model, SIGNAL(loadingEnded(bool)), this, SLOT(dataLoadingEnded(bool)));
    connect(model, SIGNAL(commitStatusChanged(bool)), this, SLOT(updateCommitRollbackActions(bool)));
    connect(gridView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            model, SLOT(updateSelectiveCommitRollbackActions(QItemSelection,QItemSelection)));
    connect(model, SIGNAL(selectiveCommitStatusChanged(bool)), this, SLOT(updateSelectiveCommitRollbackActions(bool)));
    connect(model, SIGNAL(executionStarted()), gridView, SLOT(executionStarted()));
    connect(model, SIGNAL(loadingEnded(bool)), gridView, SLOT(executionEnded()));
    connect(model, SIGNAL(totalRowsAndPagesAvailable()), this, SLOT(totalRowsAndPagesAvailable()));
    connect(gridView->horizontalHeader(), SIGNAL(sectionDoubleClicked(int)), this, SLOT(columnsHeaderDoubleClicked(int)));
    connect(this, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));
    connect(model, SIGNAL(itemEditionEnded(SqlQueryItem*)), this, SLOT(adjustColumnWidth(SqlQueryItem*)));
    connect(gridView, SIGNAL(scrolledBy(int, int)), this, SLOT(syncFilterScrollPosition()));
    connect(gridView->horizontalHeader(), SIGNAL(sectionResized(int, int, int)), this, SLOT(resizeFilter(int, int, int)));
}

void DataView::initFormView()
{
    formView = new FormView();
    formWidget->layout()->addWidget(formView);
    formView->setModel(model);
    formView->setGridView(gridView);
    connect(formView, SIGNAL(commitStatusChanged()), this, SLOT(updateFormCommitRollbackActions()));
    connect(formView, SIGNAL(currentRowChanged()), this, SLOT(updateFormNavigationState()));
    updateCurrentFormViewRow();
}

void DataView::initFilter()
{
    filterEdit = new ExtLineEdit();
    filterEdit->setExpandingMinWidth(100);
    filterEdit->setExpandingMaxWidth(200);
    filterEdit->setExpanding(true);
    filterEdit->setClearButtonEnabled(true);
    filterEdit->setPlaceholderText(tr("Filter data", "data view"));
    connect(filterEdit, SIGNAL(valueErased()), this, SLOT(resetFilter()));
    connect(filterEdit, SIGNAL(returnPressed()), this, SLOT(applyFilter()));
}

void DataView::createContents()
{
    gridWidget = new QWidget();
    formWidget = new QWidget();
    addTab(gridWidget, tr("Grid view"));
    addTab(formWidget, tr("Form view"));

    QVBoxLayout* vbox = new QVBoxLayout();
    gridWidget->setLayout(vbox);

    vbox = new QVBoxLayout();
    formWidget->setLayout(vbox);

    gridToolBar = new QToolBar();
    formToolBar = new QToolBar();
    gridWidget->layout()->addWidget(gridToolBar);
    formWidget->layout()->addWidget(formToolBar);

    createFilterPanel();
    gridWidget->layout()->addWidget(perColumnAreaParent);

    THEME_TUNER->manageCompactLayout({
                                         gridWidget,
                                         formWidget
                                    });

#ifdef Q_OS_MACX
    QStyle *fusion = QStyleFactory::create("Fusion");
    gridToolBar->setStyle(fusion);
    formToolBar->setStyle(fusion);
#endif

    gridView = new SqlQueryView();
    gridView->setCornerButtonEnabled(true);
    gridView->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    gridWidget->layout()->addWidget(gridView);
}

void DataView::createFilterPanel()
{
    perColumnAreaParent = new QWidget();
    perColumnAreaParent->setVisible(false);
    perColumnAreaParent->setLayout(new QHBoxLayout());
    perColumnAreaParent->layout()->setSpacing(0);
    perColumnAreaParent->layout()->setContentsMargins(0, 0, 0, 0);
    perColumnAreaParent->setFixedHeight(0);

    filterLeftSpacer = new QWidget();
    perColumnAreaParent->layout()->addWidget(filterLeftSpacer);

    perColumnFilterArea = new QScrollArea();
    perColumnFilterArea->setFixedHeight(0);
    perColumnFilterArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    perColumnFilterArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    perColumnFilterArea->setFrameShape(QFrame::NoFrame);
    perColumnWidget = new QWidget();
    perColumnWidget->setLayout(new QHBoxLayout());
    perColumnWidget->layout()->setSizeConstraint(QLayout::SetFixedSize);
    perColumnWidget->layout()->setSpacing(0);
    perColumnWidget->layout()->setContentsMargins(0, 0, 0, 0);
    perColumnWidget->setAutoFillBackground(true);
    perColumnWidget->setBackgroundRole(QPalette::Window);
    perColumnFilterArea->setWidget(perColumnWidget);
    perColumnAreaParent->layout()->addWidget(perColumnFilterArea);

    filterRightSpacer = new QWidget();
    perColumnAreaParent->layout()->addWidget(filterRightSpacer);
}

void DataView::initPageEdit()
{
    pageEdit = new ExtLineEdit();
    pageValidator = new IntValidator(1, 1, pageEdit);
    pageValidator->setDefaultValue(1);
    pageEdit->setAlignment(Qt::AlignCenter);
    pageEdit->setValidator(pageValidator);
    pageEdit->setExpanding(true);
    pageEdit->setExpandingMinWidth(20);
    connect(pageEdit, SIGNAL(editingFinished()), this, SLOT(pageEntered()));
}

void DataView::initWidgetCover()
{
    widgetCover = new WidgetCover(this);
    widgetCover->initWithProgressBarOnly("%v / %m");
    connect(model, SIGNAL(aboutToCommit(int)), this, SLOT(coverForGridCommit(int)));
    connect(model, SIGNAL(committingStepFinished(int)), this, SLOT(updateGridCommitCover(int)));
    connect(model, SIGNAL(commitFinished()), this, SLOT(hideGridCommitCover()));
}

void DataView::createActions()
{
    bool rowInserting = model->features().testFlag(SqlQueryModel::INSERT_ROW);
    bool rowDeleting = model->features().testFlag(SqlQueryModel::DELETE_ROW);

    // Grid actions
    createAction(REFRESH_DATA, ICONS.RELOAD, tr("Refresh table data", "data view"), this, SLOT(refreshData()), gridToolBar, gridView);
    gridToolBar->addSeparator();
    if (rowInserting)
    {
        gridToolBar->addAction(gridView->getAction(SqlQueryView::INSERT_ROW));
        attachActionInMenu(gridView->getAction(SqlQueryView::INSERT_ROW), gridView->getAction(SqlQueryView::INSERT_MULTIPLE_ROWS), gridToolBar);
        addSeparatorInMenu(gridView->getAction(SqlQueryView::INSERT_ROW), gridToolBar);
        for (Action act : {INSERT_ROW_BEFORE, INSERT_ROW_AFTER, INSERT_ROW_AT_END})
            attachActionInMenu(gridView->getAction(SqlQueryView::INSERT_ROW), staticActions[act], gridToolBar);
    }

    if (rowDeleting)
        gridToolBar->addAction(gridView->getAction(SqlQueryView::DELETE_ROW));

    gridToolBar->addAction(gridView->getAction(SqlQueryView::COMMIT));
    gridToolBar->addAction(gridView->getAction(SqlQueryView::ROLLBACK));
    gridToolBar->addSeparator();
    createAction(FIRST_PAGE, ICONS.PAGE_FIRST, tr("First page", "data view"), this, SLOT(firstPage()), gridToolBar);
    createAction(PREV_PAGE, ICONS.PAGE_PREV, tr("Previous page", "data view"), this, SLOT(prevPage()), gridToolBar);
    actionMap[PAGE_EDIT] = gridToolBar->addWidget(pageEdit);
    createAction(NEXT_PAGE, ICONS.PAGE_NEXT, tr("Next page", "data view"), this, SLOT(nextPage()), gridToolBar);
    createAction(LAST_PAGE, ICONS.PAGE_LAST, tr("Last page", "data view"), this, SLOT(lastPage()), gridToolBar);
    gridToolBar->addSeparator();
    if (model->features().testFlag(SqlQueryModel::FILTERING))
        createFilteringActions();

    actionMap[GRID_TOTAL_ROWS] = gridToolBar->addWidget(rowCountLabel);

    noConfigShortcutActions << GRID_TOTAL_ROWS << FILTER_VALUE;

    createAction(SELECTIVE_COMMIT, ICONS.COMMIT, tr("Commit changes for selected cells", "data view"), this, SLOT(selectiveCommitGrid()), this);
    createAction(SELECTIVE_ROLLBACK, ICONS.ROLLBACK, tr("Rollback changes for selected cells", "data view"), this, SLOT(selectiveRollbackGrid()), this);
    createAction(SHOW_GRID_VIEW, tr("Show grid view of results", "data view"), this, SLOT(showGridView()), this);
    createAction(SHOW_FORM_VIEW, tr("Show form view of results", "data view"), this, SLOT(showFormView()), this);

    connect(gridView, SIGNAL(requestForRowInsert()), this, SLOT(insertRow()));
    connect(gridView, SIGNAL(requestForMultipleRowInsert()), this, SLOT(insertMultipleRows()));
    connect(gridView, SIGNAL(requestForRowDelete()), this, SLOT(deleteRow()));


    // Form view actions
    if (rowInserting)
        formToolBar->addAction(formView->getAction(FormView::INSERT_ROW));

    if (rowDeleting)
        formToolBar->addAction(formView->getAction(FormView::DELETE_ROW));

    if (rowInserting || rowDeleting)
        formToolBar->addSeparator();

    formToolBar->addAction(formView->getAction(FormView::COMMIT));
    formToolBar->addAction(formView->getAction(FormView::ROLLBACK));
    formToolBar->addSeparator();
    formToolBar->addAction(formView->getAction(FormView::FIRST_ROW));
    formToolBar->addAction(formView->getAction(FormView::PREV_ROW));
    formToolBar->addAction(formView->getAction(FormView::NEXT_ROW));
    formToolBar->addAction(formView->getAction(FormView::LAST_ROW));
    formToolBar->addSeparator();
    actionMap[FORM_TOTAL_ROWS] = formToolBar->addWidget(formViewRowCountLabel);
    formToolBar->addSeparator();
    actionMap[FORM_CURRENT_ROW] = formToolBar->addWidget(formViewCurrentRowLabel);

    noConfigShortcutActions << FORM_TOTAL_ROWS;

    connect(formView, SIGNAL(requestForCommit()), this, SLOT(commitForm()));
    connect(formView, SIGNAL(requestForRollback()), this, SLOT(rollbackForm()));
    connect(formView, SIGNAL(requestForFirstRow()), this, SLOT(firstRow()));
    connect(formView, SIGNAL(requestForPrevRow()), this, SLOT(prevRow()));
    connect(formView, SIGNAL(requestForNextRow()), this, SLOT(nextRow()));
    connect(formView, SIGNAL(requestForLastRow()), this, SLOT(lastRow()));
    connect(formView, SIGNAL(requestForRowInsert()), this, SLOT(insertRow()));
    connect(formView, SIGNAL(requestForRowDelete()), this, SLOT(deleteRow()));

    // Actions for grid menu only
    gridView->addAdditionalAction(staticActions[TABS_ON_TOP]);
    gridView->addAdditionalAction(staticActions[TABS_AT_BOTTOM]);
    connect(staticActions[TABS_ON_TOP], SIGNAL(triggered()), this, SLOT(updateTabsMode()));
    connect(staticActions[TABS_AT_BOTTOM], SIGNAL(triggered()), this, SLOT(updateTabsMode()));
}

void DataView::setupDefShortcuts()
{
    // Widget context
    setShortcutContext({REFRESH_DATA, SHOW_GRID_VIEW, SHOW_FORM_VIEW}, Qt::WidgetWithChildrenShortcut);

    BIND_SHORTCUTS(DataView, Action);
}

void DataView::resizeColumnsInitiallyToContents()
{
    SqlQueryModel *model = gridView->getModel();
    int cols = model->columnCount();
    gridView->setIgnoreColumnWidthChanges(true);
    gridView->resizeColumnsToContents();
    int wd;
    int desiredWidth = -1;
    for (int i = 0; i < cols ; i++)
    {
        desiredWidth = model->getDesiredColumnWidth(i);
        wd = gridView->columnWidth(i);

        if (desiredWidth > -1 && wd != desiredWidth)
        {
            gridView->setColumnWidth(i, desiredWidth);
            continue;
        }

        int headerMinSize = qMax(gridView->horizontalHeader()->sizeHintForColumn(i), 60);
        if (wd > CFG_UI.General.MaxInitialColumnWith.get())
            gridView->setColumnWidth(i, CFG_UI.General.MaxInitialColumnWith.get());
        else if (wd < headerMinSize)
            gridView->setColumnWidth(i, headerMinSize);
    }
    gridView->setIgnoreColumnWidthChanges(false);
}

void DataView::createStaticActions()
{
    // Tabs position actions
    staticActions[TABS_ON_TOP] = new ExtAction(ICONS.TABS_ON_TOP, tr("Tabs on top", "data view"), MainWindow::getInstance());
    staticActions[TABS_AT_BOTTOM] = new ExtAction(ICONS.TABS_AT_BOTTOM, tr("Tabs at bottom", "data view"), MainWindow::getInstance());

    staticActionGroups[ActionGroup::TABS_POSITION] = new QActionGroup(MainWindow::getInstance());
    staticActionGroups[ActionGroup::TABS_POSITION]->addAction(staticActions[TABS_ON_TOP]);
    staticActionGroups[ActionGroup::TABS_POSITION]->addAction(staticActions[TABS_AT_BOTTOM]);

    connect(staticActions[TABS_ON_TOP], &QAction::triggered, [=]()
    {
        tabsPosition = TabsPosition::TOP;
        CFG_UI.General.DataViewTabs.set("TOP");
    });
    connect(staticActions[TABS_AT_BOTTOM], &QAction::triggered, [=]()
    {
        tabsPosition = TabsPosition::BOTTOM;
        CFG_UI.General.DataViewTabs.set("BOTTOM");
    });

    staticActions[TABS_ON_TOP]->setCheckable(true);
    staticActions[TABS_AT_BOTTOM]->setCheckable(true);
    if (tabsPosition == TabsPosition::TOP)
        staticActions[TABS_ON_TOP]->setChecked(true);
    else
        staticActions[TABS_AT_BOTTOM]->setChecked(true);

    // Insert row positioning
    staticActions[INSERT_ROW_BEFORE] = new ExtAction(tr("Place new rows above selected row", "data view"), MainWindow::getInstance());
    staticActions[INSERT_ROW_AFTER] = new ExtAction(tr("Place new rows below selected row", "data view"), MainWindow::getInstance());
    staticActions[INSERT_ROW_AT_END] = new ExtAction(tr("Place new rows at the end of the data view", "data view"), MainWindow::getInstance());

    staticActionGroups[ActionGroup::INSERT_ROW_POSITIONING] = new QActionGroup(MainWindow::getInstance());
    staticActionGroups[ActionGroup::INSERT_ROW_POSITIONING]->addAction(staticActions[INSERT_ROW_BEFORE]);
    staticActionGroups[ActionGroup::INSERT_ROW_POSITIONING]->addAction(staticActions[INSERT_ROW_AFTER]);
    staticActionGroups[ActionGroup::INSERT_ROW_POSITIONING]->addAction(staticActions[INSERT_ROW_AT_END]);

    connect(staticActions[INSERT_ROW_BEFORE], &QAction::triggered, [=]()
    {
        CFG_UI.General.InsertRowPlacement.set(Cfg::BEFORE_CURRENT);
    });
    connect(staticActions[INSERT_ROW_AFTER], &QAction::triggered, [=]()
    {
        CFG_UI.General.InsertRowPlacement.set(Cfg::AFTER_CURRENT);
    });
    connect(staticActions[INSERT_ROW_AT_END], &QAction::triggered, [=]()
    {
        CFG_UI.General.InsertRowPlacement.set(Cfg::AT_THE_END);
    });

    staticActions[INSERT_ROW_BEFORE]->setCheckable(true);
    staticActions[INSERT_ROW_AFTER]->setCheckable(true);
    staticActions[INSERT_ROW_AT_END]->setCheckable(true);
    switch (static_cast<Cfg::InsertRowPlacement>(CFG_UI.General.InsertRowPlacement.get()))
    {
        case Cfg::BEFORE_CURRENT:
            staticActions[INSERT_ROW_BEFORE]->setChecked(true);
            break;
        case Cfg::AFTER_CURRENT:
            staticActions[INSERT_ROW_AFTER]->setChecked(true);
            break;
        case Cfg::AT_THE_END:
            staticActions[INSERT_ROW_AT_END]->setChecked(true);
            break;
    }
}

void DataView::loadTabsMode()
{
    QString valString = CFG_UI.General.DataViewTabs.get();
    if (valString == "TOP")
        tabsPosition = TabsPosition::TOP;
    else if (valString == "BOTTOM")
        tabsPosition = TabsPosition::BOTTOM;
}

void DataView::goToFormRow(IndexModifier idxMod)
{
    if (formView->isModified())
        formView->copyDataToGrid();

    int row = gridView->getCurrentIndex().row();

    switch (idxMod)
    {
        case IndexModifier::FIRST:
            row = 0;
            break;
        case IndexModifier::PREV:
            row--;
            break;
        case IndexModifier::NEXT:
            row++;
            break;
        case IndexModifier::LAST:
            row = model->rowCount() - 1;
            break;
    }

    QModelIndex newRowIdx = model->index(row, 0);
    if (!newRowIdx.isValid())
        return;

    gridView->setCurrentIndex(newRowIdx);
    formView->updateFromGrid();
    updateCurrentFormViewRow();
}

void DataView::setNavigationState(bool enabled)
{
    navigationState = enabled;
    updateNavigationState();
    setFormViewEnabled(enabled);
}

void DataView::updateNavigationState()
{
    updateGridNavigationState();
    updateFormNavigationState();
}

void DataView::updateGridNavigationState()
{
    int page = model->getCurrentPage();
    bool prevResultsAvailable = page > 0;
    bool nextResultsAvailable = (page + 1) < model->getTotalPages();
    bool reloadResultsAvailable = model->canReload();
    bool pageNumEditAvailable = (prevResultsAvailable || nextResultsAvailable);

    actionMap[PAGE_EDIT]->setEnabled(navigationState && totalPagesAvailable && pageNumEditAvailable);
    actionMap[REFRESH_DATA]->setEnabled(navigationState && reloadResultsAvailable);
    actionMap[NEXT_PAGE]->setEnabled(navigationState && totalPagesAvailable && nextResultsAvailable);
    actionMap[LAST_PAGE]->setEnabled(navigationState && totalPagesAvailable && nextResultsAvailable);
    actionMap[PREV_PAGE]->setEnabled(navigationState && totalPagesAvailable && prevResultsAvailable);
    actionMap[FIRST_PAGE]->setEnabled(navigationState && totalPagesAvailable && prevResultsAvailable);
}

void DataView::updateFormNavigationState()
{
    int row = gridView->getCurrentIndex().row();
    int lastRow = model->rowCount() - 1;
    bool nextRowAvailable = row < lastRow;
    bool prevRowAvailable = row > 0;

    formView->getAction(FormView::NEXT_ROW)->setEnabled(navigationState && nextRowAvailable);
    formView->getAction(FormView::PREV_ROW)->setEnabled(navigationState && prevRowAvailable);

    // We changed row in form view, this one might be already modified and be capable for commit/rollback
    updateFormCommitRollbackActions();
}

void DataView::updateFormCommitRollbackActions()
{
    bool enabled = formView->isModified();
    formView->getAction(FormView::COMMIT)->setEnabled(enabled);
    formView->getAction(FormView::ROLLBACK)->setEnabled(enabled);
    uncommittedForm = enabled;
}

void DataView::showGridView()
{
    setCurrentIndex(0);
}

void DataView::showFormView()
{
    setCurrentIndex(1);
    updateCurrentFormViewRow();
}

void DataView::updateTabsMode()
{
    switch (tabsPosition)
    {
        case DataView::TabsPosition::TOP:
            setTabPosition(TabPosition::North);
            break;
        case DataView::TabsPosition::BOTTOM:
            setTabPosition(TabPosition::South);
            break;
    }
}

void DataView::setActionIcon(QAction *action, const QIcon &icon, QToolBar *toolbar)
{
    action->setIcon(icon);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    // In Qt 6, setting the action icon is not enough, use brute force
    QToolButton* button = dynamic_cast<QToolButton*>(toolbar->widgetForAction(action));
    button->setIcon(icon);
#else
    Q_UNUSED(toolbar);
#endif
}

void DataView::filterModeSelected()
{
    QAction* modeAction = dynamic_cast<QAction*>(sender());
    filterMode = static_cast<FilterMode>(modeAction->property(DATA_VIEW_FILTER_PROP).toInt());
    setActionIcon(actionMap[FILTER], modeAction->icon(), gridToolBar);
}

void DataView::coverForGridCommit(int total)
{
    if (total <= 3)
        return;

    widgetCover->displayProgress(total);
    widgetCover->show();
    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
}

void DataView::updateGridCommitCover(int value)
{
    if (!widgetCover->isVisible() || (value % 10) != 0)
        return;

    widgetCover->setProgress(value);
    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
}

void DataView::hideGridCommitCover()
{
    if (!widgetCover->isVisible())
        return;

    widgetCover->hide();
    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
}

void DataView::adjustColumnWidth(SqlQueryItem* item)
{
    if (!item)
        return;

    int col = item->column();
    if (model->getDesiredColumnWidth(col) > -1)
        return;

    if (!CFG_UI.General.EnlargeColumnForValue.get())
        return;

    gridView->resizeColumnToContents(col);
    if (gridView->columnWidth(col) > CFG_UI.General.MaxInitialColumnWith.get())
        gridView->setColumnWidth(col, CFG_UI.General.MaxInitialColumnWith.get());
}

void DataView::syncFilterScrollPosition()
{
    perColumnFilterArea->horizontalScrollBar()->setValue(gridView->horizontalScrollBar()->value());
}

void DataView::resizeFilter(int section, int oldSize, int newSize)
{
    UNUSED(oldSize);
    if (!model->features().testFlag(SqlQueryModel::FILTERING))
        return;

    if (filterInputs.isEmpty())
        return;

    if (filterInputs.size() <= section)
    {
        qCritical() << "Tried to adjust per-column filter input edit according to resized value, but section index is out of bounds:"
                    << section << ", while edit widgets count is:" << filterInputs.size();
        return;
    }

    filterInputs[section]->setFixedWidth(newSize);
}

void DataView::togglePerColumnFiltering()
{
    bool enable = actionMap[FILTER_PER_COLUMN]->isChecked();

    filterEdit->setEnabled(!enable);
    if (actionMap[FILTER_SQL]->isChecked())
        actionMap[FILTER_STRING]->setChecked(true);

    actionMap[FILTER_SQL]->setEnabled(!enable);
    perColumnAreaParent->setVisible(enable);

    recreateFilterInputs();
}

void DataView::updateCommitRollbackActions(bool enabled)
{
    gridView->getAction(SqlQueryView::COMMIT)->setEnabled(enabled);
    gridView->getAction(SqlQueryView::ROLLBACK)->setEnabled(enabled);
    uncommittedGrid = enabled;
}

void DataView::updateSelectiveCommitRollbackActions(bool enabled)
{
    actionMap[SELECTIVE_COMMIT]->setEnabled(enabled);
    actionMap[SELECTIVE_ROLLBACK]->setEnabled(enabled);
}

void DataView::goToPage(const QString& pageStr)
{
    bool ok;
    int page = pageStr.toInt(&ok);
    if (!ok)
        return;

    page--; // Converting from visual page representation to logical

    if (page == model->getCurrentPage(true))
        return;

    // We need to get this synchronized against event loop, cause changeing action status (probably) calls event loop update,
    // so this method was sometimes called twice at the time (until setResultsNavigationState() call below),
    // but the page in results model wasn't updated yet. We cannot simply move setResultsNavigationState() below gotoPage(),
    // because we need to disable actions, before model returns from execution, so it can re-enable those actions.
    // This method was called twice, because QLineEdit::editionFinished (the filter) signal is emitted twice:
    // - because enter was pressed
    // - because edit lost its focus (which happens after enter was hit), at least it looks like it
    if (!manualPageChangeMutex.tryLock())
        return;

    setNavigationState(false);
    model->gotoPage(page);
    manualPageChangeMutex.unlock();
}

void DataView::updatePageEdit()
{
    int page = model->getCurrentPage()+1;
    QString text = QString::number(page);
    int totalPages = model->getTotalPages();
    pageEdit->setText(text);
    pageEdit->setToolTip(QObject::tr("Total pages available: %1").arg(totalPages));
    pageValidator->setTop(totalPages);
    pageValidator->setDefaultValue(page);
    updateCurrentFormViewRow();
}

void DataView::updateResultsCount(int resultsCount)
{
    if (resultsCount >= 0)
    {
        QString msg = QObject::tr("Total rows loaded: %1").arg(resultsCount);
        rowCountLabel->setText(msg);
        formViewRowCountLabel->setText(msg);
        rowCountLabel->setToolTip(QString());
        formViewRowCountLabel->setToolTip(QString());
    }
    else
    {
        rowCountLabel->setText("        "); // this might seem weird, but if it's not a wide, whitespace string, then icon is truncated from right side
        formViewRowCountLabel->setText("        ");
        rowCountLabel->setMovie(ICONS.LOADING);
        formViewRowCountLabel->setMovie(ICONS.LOADING);

        static QString loadingMsg = tr("Total number of rows is being counted.\nBrowsing other pages will be possible after the row counting is done.");
        rowCountLabel->setToolTip(loadingMsg);
        formViewRowCountLabel->setToolTip(loadingMsg);
    }
}

void DataView::updateCurrentFormViewRow()
{
    int rowsPerPage = CFG_UI.General.NumberOfRowsPerPage.get();
    int page = gridView->getModel()->getCurrentPage();
    int row = rowsPerPage * page + 1 + gridView->getCurrentIndex().row();
    formViewCurrentRowLabel->setText(tr("Row: %1").arg(row));
}

void DataView::setFormViewEnabled(bool enabled)
{
    if (!enabled)
        setCurrentIndex(0);

    setTabEnabled(1, enabled);
}

bool DataView::isUncommitted() const
{
    return uncommittedGrid || uncommittedForm;
}

void DataView::dataLoadingEnded(bool successful)
{
    if (successful)
    {
        updatePageEdit();
        resizeColumnsInitiallyToContents();
        recreateFilterInputs();
    }

    setNavigationState(true);
}

void DataView::executionSuccessful()
{
    updateResultsCount(-1);
}

void DataView::totalRowsAndPagesAvailable()
{
    updateResultsCount(model->getTotalRowsReturned());
    totalPagesAvailable = true;
    updatePageEdit();
    updateNavigationState();
}

void DataView::refreshData(bool keepFocus)
{
    totalPagesAvailable = false;
    if (keepFocus)
        model->rememberFocusedCell();
    else
        model->forgetFocusedCell();

    setNavigationState(false);
    model->executeQuery(!keepFocus);
}

void DataView::insertRow()
{
    if (!model->features().testFlag(SqlQueryModel::INSERT_ROW))
        return;

    model->addNewRow();
    initFormViewForNewRow();
    formView->updateFromGrid();
    updateCurrentFormViewRow();
    formViewFocusFirstEditor();
}

void DataView::insertMultipleRows()
{
    if (!model->features().testFlag(SqlQueryModel::INSERT_ROW))
        return;

    model->addMultipleRows();
    formView->updateFromGrid();
    updateCurrentFormViewRow();
    formViewFocusFirstEditor();
}

void DataView::deleteRow()
{
    if (!model->features().testFlag(SqlQueryModel::DELETE_ROW))
        return;

    model->deleteSelectedRows();
    formView->updateFromGrid();
    updateCurrentFormViewRow();
    formViewFocusFirstEditor();
}

void DataView::commitGrid()
{
    model->commit();
}

void DataView::rollbackGrid()
{
    model->rollback();
}

void DataView::selectiveCommitGrid()
{
    QList<SqlQueryItem*> selectedItems = gridView->getSelectedItems();
    model->commit(selectedItems);
}

void DataView::selectiveRollbackGrid()
{
    QList<SqlQueryItem*> selectedItems = gridView->getSelectedItems();
    model->rollback(selectedItems);
}

void DataView::firstPage()
{
    setNavigationState(false);
    model->firstPage();
}

void DataView::prevPage()
{
    setNavigationState(false);
    model->prevPage();
}

void DataView::nextPage()
{
    setNavigationState(false);
    model->nextPage();
}

void DataView::lastPage()
{
    setNavigationState(false);
    model->lastPage();
}

void DataView::pageEntered()
{
    goToPage(pageEdit->text());
}

void DataView::applyFilter()
{
    if (!model->features().testFlag(SqlQueryModel::Feature::FILTERING))
    {
        qWarning() << "Tried to apply filter on model that doesn't support it.";
        return;
    }

    if (actionMap[FILTER_PER_COLUMN]->isChecked())
    {
        filterValues.clear();
        for (QLineEdit* edit : filterInputs)
            filterValues << edit->text();

        if (filterValues.join("").isEmpty())
        {
            model->resetFilter();
            return;
        }

        switch (filterMode)
        {
            case DataView::FilterMode::STRING:
                model->applyStringFilter(filterValues);
                break;
            case DataView::FilterMode::SQL:
                // Should never happen.
                qWarning() << "Requested to filter by SQL for filtering per-column. This should not be possible.";
                break;
            case DataView::FilterMode::REGEXP:
                model->applyRegExpFilter(filterValues);
                break;
            case FilterMode::EXACT:
                model->applyStrictFilter(filterValues);
                break;
        }
    }
    else
    {
        QString value = filterEdit->text();
        switch (filterMode)
        {
            case DataView::FilterMode::STRING:
                model->applyStringFilter(value);
                break;
            case DataView::FilterMode::SQL:
                model->applySqlFilter(value);
                break;
            case DataView::FilterMode::REGEXP:
                model->applyRegExpFilter(value);
                break;
            case DataView::FilterMode::EXACT:
                model->applyStrictFilter(value);
                break;
        }
    }
}

void DataView::resetFilter()
{
    if (!model->features().testFlag(SqlQueryModel::Feature::FILTERING))
    {
        qWarning() << "Tried to reset filter on model that doesn't support it.";
        return;
    }

    model->resetFilter();
}

void DataView::commitForm()
{
    formView->copyDataToGrid();
    gridView->selectRow(formView->getCurrentRow());
    selectiveCommitGrid();
    formView->updateFromGrid();
    formViewFocusFirstEditor();
}

void DataView::rollbackForm()
{
    formView->copyDataToGrid();
    gridView->selectRow(formView->getCurrentRow());
    selectiveRollbackGrid();
    formView->updateFromGrid();
    updateCurrentFormViewRow();
    formViewFocusFirstEditor();
}

void DataView::firstRow()
{
    goToFormRow(IndexModifier::FIRST);
    formViewFocusFirstEditor();
}

void DataView::prevRow()
{
    goToFormRow(IndexModifier::PREV);
    formViewFocusFirstEditor();
}

void DataView::nextRow()
{
    goToFormRow(IndexModifier::NEXT);
    formViewFocusFirstEditor();
}

void DataView::lastRow()
{
    goToFormRow(IndexModifier::LAST);
    formViewFocusFirstEditor();
}

void DataView::initFormViewForNewRow()
{
    if (currentWidget() != formWidget)
        return;

    int row = gridView->getCurrentIndex().row();
    for (SqlQueryItem* item : getModel()->getRow(row))
    {
        if (item->getColumn()->isAutoIncr())
            continue;

        item->setValue("");
    }
}

void DataView::formViewFocusFirstEditor()
{
    if (currentWidget() == formWidget)
        formView->focusFirstEditor();
}

void DataView::recreateFilterInputs()
{
    if (!model->features().testFlag(SqlQueryModel::FILTERING))
        return;

    qApp->processEvents();

    for (ExtLineEdit*& edit : filterInputs)
        delete edit;

    filterInputs.clear();

    filterLeftSpacer->setFixedSize(gridView->verticalHeader()->width() + 1, 1);

    ExtLineEdit* edit = nullptr;
    for (int i = 0, total = gridView->horizontalHeader()->count(); i < total; ++i)
    {
        edit = new ExtLineEdit(perColumnWidget);
        edit->setPlaceholderText(tr("Filter"));
        edit->setClearButtonEnabled(true);
        edit->setFixedWidth(gridView->columnWidth(i));
        edit->setToolTip(tr("Hit Enter key or press \"Apply filter\" button on toolbar to apply new value."));
        if (filterValues.size() > i)
            edit->setText(filterValues[i]);

        connect(edit, SIGNAL(returnPressed()), this, SLOT(applyFilter()));
        perColumnWidget->layout()->addWidget(edit);
        filterInputs << edit;
    }

    int rightSpacerWd = gridView->verticalScrollBar()->isVisible() ? gridView->verticalScrollBar()->width() : 0;
    filterRightSpacer->setFixedSize(rightSpacerWd + 1, 1);

    perColumnAreaParent->setFixedWidth(gridView->width());

    if (edit)
    {
        int hg = edit->sizeHint().height();
        perColumnFilterArea->setFixedHeight(hg);
        perColumnAreaParent->setFixedHeight(hg);
    }

    qApp->processEvents();

    syncFilterScrollPosition();
}

void DataView::createFilteringActions()
{
    createAction(FILTER_STRING, ICONS.APPLY_FILTER_TXT, tr("Filter by text (if contains)", "data view"), this, SLOT(filterModeSelected()), this);
    createAction(FILTER_EXACT, ICONS.APPLY_FILTER_TXT_STRICT, tr("Filter strictly by text (if equals)", "data view"), this, SLOT(filterModeSelected()), this);
    createAction(FILTER_REGEXP, ICONS.APPLY_FILTER_RE, tr("Filter by the Regular Expression", "data view"), this, SLOT(filterModeSelected()), this);
    createAction(FILTER_SQL, ICONS.APPLY_FILTER_SQL, tr("Filter by SQL expression", "data view"), this, SLOT(filterModeSelected()), this);

    actionMap[FILTER_STRING]->setProperty(DATA_VIEW_FILTER_PROP, static_cast<int>(FilterMode::STRING));
    actionMap[FILTER_EXACT]->setProperty(DATA_VIEW_FILTER_PROP, static_cast<int>(FilterMode::EXACT));
    actionMap[FILTER_REGEXP]->setProperty(DATA_VIEW_FILTER_PROP, static_cast<int>(FilterMode::REGEXP));
    actionMap[FILTER_SQL]->setProperty(DATA_VIEW_FILTER_PROP, static_cast<int>(FilterMode::SQL));

    QActionGroup* filterGroup = new QActionGroup(gridToolBar);
    filterGroup->addAction(actionMap[FILTER_STRING]);
    filterGroup->addAction(actionMap[FILTER_EXACT]);
    filterGroup->addAction(actionMap[FILTER_SQL]);
    filterGroup->addAction(actionMap[FILTER_REGEXP]);

    actionMap[FILTER_STRING]->setCheckable(true);
    actionMap[FILTER_EXACT]->setCheckable(true);
    actionMap[FILTER_REGEXP]->setCheckable(true);
    actionMap[FILTER_SQL]->setCheckable(true);
    actionMap[FILTER_STRING]->setChecked(true);

    createAction(FILTER_PER_COLUMN, tr("Show filter inputs per column", "data view"), this, SLOT(togglePerColumnFiltering()), this);
    actionMap[FILTER_PER_COLUMN]->setCheckable(true);

    actionMap[FILTER_VALUE] = gridToolBar->addWidget(filterEdit);
    createAction(FILTER, tr("Apply filter", "data view"), this, SLOT(applyFilter()), gridToolBar);
    attachActionInMenu(FILTER, actionMap[FILTER_STRING], gridToolBar);
    attachActionInMenu(FILTER, actionMap[FILTER_EXACT], gridToolBar);
    attachActionInMenu(FILTER, actionMap[FILTER_REGEXP], gridToolBar);
    attachActionInMenu(FILTER, actionMap[FILTER_SQL], gridToolBar);
    addSeparatorInMenu(FILTER, gridToolBar);
    attachActionInMenu(FILTER, actionMap[FILTER_PER_COLUMN], gridToolBar);
    gridToolBar->addSeparator();

    setActionIcon(actionMap[FILTER], actionMap[FILTER_STRING]->icon(), gridToolBar);

    gridView->getHeaderContextMenu()->addSeparator();
    gridView->getHeaderContextMenu()->addAction(actionMap[FILTER_PER_COLUMN]);
}


void DataView::columnsHeaderDoubleClicked(int columnIdx)
{
    model->changeSorting(columnIdx);
}

void DataView::tabChanged(int newIndex)
{
    switch (newIndex)
    {
        case 0:
        {
            formView->copyDataToGrid();
            gridView->setFocus();
            break;
        }
        case 1:
        {
            if (!gridView->getCurrentIndex().isValid() && model->rowCount() > 0)
                gridView->setCurrentRow(0);

            formView->updateFromGrid();
            updateCurrentFormViewRow();
            break;
        }
    }
}

FormView* DataView::getFormView() const
{
    return formView;
}

SqlQueryModel* DataView::getModel() const
{
    return model;
}

QToolBar* DataView::getToolBar(int toolbar) const
{
    switch (static_cast<ToolBar>(toolbar))
    {
        case TOOLBAR_GRID:
            return gridToolBar;
        case TOOLBAR_FORM:
            return formToolBar;
    }
    return nullptr;
}

void DataView::staticInit()
{
    tabsPosition = TabsPosition::TOP;
    loadTabsMode();
    createStaticActions();
}

void DataView::insertAction(ExtActionPrototype* action, DataView::ToolBar toolbar)
{
    return ExtActionContainer::insertAction<DataView>(action, toolbar);
}

void DataView::insertActionBefore(ExtActionPrototype* action, DataView::Action beforeAction, DataView::ToolBar toolbar)
{
    return ExtActionContainer::insertActionBefore<DataView>(action, beforeAction, toolbar);
}

void DataView::insertActionAfter(ExtActionPrototype* action, DataView::Action afterAction, DataView::ToolBar toolbar)
{
    return ExtActionContainer::insertActionAfter<DataView>(action, afterAction, toolbar);
}

void DataView::removeAction(ExtActionPrototype* action, DataView::ToolBar toolbar)
{
    ExtActionContainer::removeAction<DataView>(action, toolbar);
}

SqlQueryView* DataView::getGridView() const
{
    return gridView;
}

TYPE_OF_QHASH qHash(DataView::ActionGroup action)
{
    return static_cast<TYPE_OF_QHASH>(action);
}
