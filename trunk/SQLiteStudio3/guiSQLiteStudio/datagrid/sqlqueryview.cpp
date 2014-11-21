#include "sqlqueryview.h"
#include "sqlqueryitemdelegate.h"
#include "sqlquerymodel.h"
#include "sqlqueryitem.h"
#include "common/widgetcover.h"
#include "tsvserializer.h"
#include "iconmanager.h"
#include "common/unused.h"
#include "common/extaction.h"
#include "multieditor/multieditor.h"
#include "multieditor/multieditordialog.h"
#include "uiconfig.h"
#include "dialogs/sortdialog.h"
#include <QHeaderView>
#include <QPushButton>
#include <QProgressBar>
#include <QGridLayout>
#include <QDebug>
#include <QFocusEvent>
#include <QApplication>
#include <QClipboard>
#include <QAction>
#include <QMenu>

CFG_KEYS_DEFINE(SqlQueryView)

SqlQueryView::SqlQueryView(QWidget *parent) :
    QTableView(parent)
{
    init();
}

SqlQueryView::~SqlQueryView()
{
    delete itemDelegate;
}

QList<SqlQueryItem*> SqlQueryView::getSelectedItems()
{
    QList<SqlQueryItem*> items;
    QModelIndexList idxList = selectionModel()->selectedIndexes();
    QModelIndex currIdx = getCurrentIndex();
    if (!idxList.contains(currIdx) && currIdx.isValid())
        idxList << currIdx;

    if (idxList.size() == 0)
        return items;

    qSort(idxList);
    const SqlQueryModel* model = dynamic_cast<const SqlQueryModel*>(idxList.first().model());
    foreach (const QModelIndex& idx, idxList)
        items << model->itemFromIndex(idx);

    return items;
}

SqlQueryItem* SqlQueryView::getCurrentItem()
{
    QModelIndex idx = getCurrentIndex();
    if (!idx.isValid())
        return nullptr;

    return getModel()->itemFromIndex(idx);
}

SqlQueryModel* SqlQueryView::getModel()
{
    return dynamic_cast<SqlQueryModel*>(model());
}

void SqlQueryView::setModel(QAbstractItemModel* model)
{
    QTableView::setModel(model);
    connect(widgetCover, SIGNAL(cancelClicked()), getModel(), SLOT(interrupt()));
    connect(getModel(), &SqlQueryModel::commitStatusChanged, this, &SqlQueryView::updateCommitRollbackActions);
    connect(getModel(), &SqlQueryModel::sortingUpdated, this, &SqlQueryView::sortingUpdated);
}

SqlQueryItem* SqlQueryView::itemAt(const QPoint& pos)
{
    return dynamic_cast<SqlQueryItem*>(getModel()->itemFromIndex(indexAt(pos)));
}

QToolBar* SqlQueryView::getToolBar(int toolbar) const
{
    UNUSED(toolbar);
    return nullptr;
}

void SqlQueryView::addAdditionalAction(QAction* action)
{
    additionalActions << action;
}

QModelIndex SqlQueryView::getCurrentIndex() const
{
    return currentIndex();
}

void SqlQueryView::mouseDoubleClickEvent(QMouseEvent* event)
{
    SqlQueryItem* item = itemAt(event->pos());
    if (item && !handleDoubleClick(item))
        return;

    QTableView::mouseDoubleClickEvent(event);
}

void SqlQueryView::init()
{
    itemDelegate = new SqlQueryItemDelegate();
    setItemDelegate(itemDelegate);
    setMouseTracking(true);

    setContextMenuPolicy(Qt::CustomContextMenu);
    contextMenu = new QMenu(this);

    connect(this, &QWidget::customContextMenuRequested, this, &SqlQueryView::customContextMenuRequested);
    connect(CFG_UI.Fonts.DataView, SIGNAL(changed(QVariant)), this, SLOT(updateFont()));

    horizontalHeader()->setSortIndicatorShown(false);
    horizontalHeader()->setSectionsClickable(true);
    updateFont();

    setupWidgetCover();
    initActions();
    setupHeaderMenu();
}

void SqlQueryView::setupWidgetCover()
{
    widgetCover = new WidgetCover(this);
    widgetCover->initWithInterruptContainer();
}

void SqlQueryView::createActions()
{
    createAction(COPY, ICONS.ACT_COPY, tr("Copy"), this, SLOT(copy()), this);
    createAction(COPY_AS, ICONS.ACT_COPY, tr("Copy as..."), this, SLOT(copyAs()), this);
    createAction(PASTE, ICONS.ACT_PASTE, tr("Paste"), this, SLOT(paste()), this);
    createAction(PASTE_AS, ICONS.ACT_PASTE, tr("Paste as..."), this, SLOT(pasteAs()), this);
    createAction(SET_NULL, ICONS.SET_NULL, tr("Set NULL values"), this, SLOT(setNull()), this);
    createAction(ERASE, ICONS.ERASE, tr("Erase values"), this, SLOT(erase()), this);
    createAction(OPEN_VALUE_EDITOR, ICONS.OPEN_VALUE_EDITOR, tr("Edit value in editor"), this, SLOT(openValueEditor()), this);
    createAction(COMMIT, ICONS.COMMIT, tr("Commit"), this, SLOT(commit()), this);
    createAction(ROLLBACK, ICONS.ROLLBACK, tr("Rollback"), this, SLOT(rollback()), this);
    createAction(SELECTIVE_COMMIT, ICONS.COMMIT, tr("Commit selected cells"), this, SLOT(selectiveCommit()), this);
    createAction(SELECTIVE_ROLLBACK, ICONS.ROLLBACK, tr("Rollback selected cells"), this, SLOT(selectiveRollback()), this);
    createAction(SORT_DIALOG, ICONS.SORT_COLUMNS, tr("Define columns to sort by"), this, SLOT(openSortDialog()), this);
    createAction(RESET_SORTING, ICONS.SORT_RESET, tr("Remove custom sorting"), this, SLOT(resetSorting()), this);
    createAction(INSERT_ROW, ICONS.INSERT_ROW, tr("Insert row"), this, SIGNAL(requestForRowInsert()), this);
    createAction(INSERT_MULTIPLE_ROWS, ICONS.INSERT_ROWS, tr("Insert multiple rows"), this, SIGNAL(requestForMultipleRowInsert()), this);
    createAction(DELETE_ROW, ICONS.DELETE_ROW, tr("Delete selected row"), this, SIGNAL(requestForRowDelete()), this);

    actionMap[RESET_SORTING]->setEnabled(false);
}

void SqlQueryView::setupDefShortcuts()
{
    setShortcutContext({ROLLBACK, SET_NULL, ERASE, OPEN_VALUE_EDITOR, COMMIT, COPY, COPY_AS,
                       PASTE, PASTE_AS}, Qt::WidgetWithChildrenShortcut);

    BIND_SHORTCUTS(SqlQueryView, Action);
}

void SqlQueryView::setupActionsForMenu(SqlQueryItem* currentItem, const QList<SqlQueryItem*>& selectedItems)
{
    UNUSED(currentItem);

    // Selected items count
    int selCount = selectedItems.size();

    // Uncommited items count
    QList<SqlQueryItem*> uncommitedItems = getModel()->getUncommitedItems();
    int uncommitedCount = uncommitedItems.size();

    // Uncommited & selected items count
    int uncommitedSelCount = 0;
    foreach (SqlQueryItem* item, uncommitedItems)
        if (selectedItems.contains(item))
            uncommitedSelCount++;

    if (uncommitedCount > 0)
        contextMenu->addAction(actionMap[COMMIT]);

    if (uncommitedSelCount > 0)
        contextMenu->addAction(actionMap[SELECTIVE_COMMIT]);

    if (uncommitedCount > 0)
        contextMenu->addAction(actionMap[ROLLBACK]);

    if (uncommitedSelCount > 0)
        contextMenu->addAction(actionMap[SELECTIVE_ROLLBACK]);

    if (uncommitedCount > 0 && selCount > 0)
        contextMenu->addSeparator();

    if (selCount > 0)
    {
        contextMenu->addAction(actionMap[ERASE]);
        contextMenu->addAction(actionMap[SET_NULL]);
        contextMenu->addAction(actionMap[OPEN_VALUE_EDITOR]);
        contextMenu->addSeparator();
    }

    if (selCount > 0)
    {
        contextMenu->addAction(actionMap[COPY]);
        //contextMenu->addAction(actionMap[COPY_AS]); // TODO uncomment when implemented
        contextMenu->addAction(actionMap[PASTE]);
        //contextMenu->addAction(actionMap[PASTE_AS]); // TODO uncomment when implemented
    }
    if (additionalActions.size() > 0)
    {
        contextMenu->addSeparator();
        foreach (QAction* action, additionalActions)
            contextMenu->addAction(action);
    }
}

void SqlQueryView::setupHeaderMenu()
{
    horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(horizontalHeader(), &QWidget::customContextMenuRequested, this, &SqlQueryView::headerContextMenuRequested);
    headerContextMenu = new QMenu(horizontalHeader());
    headerContextMenu->addAction(actionMap[SORT_DIALOG]);
    headerContextMenu->addAction(actionMap[RESET_SORTING]);
}

bool SqlQueryView::handleDoubleClick(SqlQueryItem* item)
{
    if (item->getColumn()->dataType.getType() == DataType::BLOB)
    {
        openValueEditor(item);
        return false;
    }
    return true;
}

void SqlQueryView::updateCommitRollbackActions(bool enabled)
{
    actionMap[COMMIT]->setEnabled(enabled);
    actionMap[ROLLBACK]->setEnabled(enabled);
}

void SqlQueryView::customContextMenuRequested(const QPoint& pos)
{
    SqlQueryItem* currentItem = getCurrentItem();
    QList<SqlQueryItem*> selectedItems = getSelectedItems();

    contextMenu->clear();

    setupActionsForMenu(currentItem, selectedItems);
    emit contextMenuRequested(currentItem, selectedItems);

    if (contextMenu->actions().size() == 0)
        return;

    contextMenu->popup(viewport()->mapToGlobal(pos));
}

void SqlQueryView::headerContextMenuRequested(const QPoint& pos)
{
    headerContextMenu->popup(horizontalHeader()->mapToGlobal(pos));
}

void SqlQueryView::openSortDialog()
{
    QStringList columns;
    for (SqlQueryModelColumnPtr col : getModel()->getColumns())
        columns << col->displayName;

    SortDialog dialog(this);
    dialog.setColumns(columns);
    dialog.setSortOrder(getModel()->getSortOrder());
    if (dialog.exec() != QDialog::Accepted)
        return;

    getModel()->setSortOrder(dialog.getSortOrder());
}

void SqlQueryView::resetSorting()
{
    getModel()->setSortOrder(QueryExecutor::SortList());
}

void SqlQueryView::sortingUpdated(const QueryExecutor::SortList& sortOrder)
{
    actionMap[RESET_SORTING]->setEnabled(sortOrder.size() > 0);
}

void SqlQueryView::updateFont()
{
    QFont f = CFG_UI.Fonts.DataView.get();
    QFontMetrics fm(f);
    verticalHeader()->setDefaultSectionSize(fm.height() + 4);
}

void SqlQueryView::executionStarted()
{
    widgetCover->show();
}

void SqlQueryView::executionEnded()
{
    widgetCover->hide();
}

void SqlQueryView::setCurrentRow(int row)
{
    setCurrentIndex(model()->index(row, 0));
}

void SqlQueryView::copy()
{
    QList<SqlQueryItem*> selectedItems = getSelectedItems();
    QList<QList<SqlQueryItem*> > groupedItems = SqlQueryModel::groupItemsByRows(selectedItems);

    QStringList cells;
    QList<QStringList> rows;

    foreach (const QList<SqlQueryItem*>& itemsInRows, groupedItems)
    {
        foreach (SqlQueryItem* item, itemsInRows)
            cells << item->getFullValue().toString();

        rows << cells;
        cells.clear();
    }

    QString tsv = TsvSerializer::serialize(rows);
    qApp->clipboard()->setText(tsv);
}

void SqlQueryView::paste()
{
    QList<QStringList> deserializedRows = TsvSerializer::deserialize(qApp->clipboard()->text());

    QList<SqlQueryItem*> selectedItems = getSelectedItems();
    qSort(selectedItems);
    SqlQueryItem* topLeft = selectedItems.first();

    int columnCount = getModel()->columnCount();
    int rowCount = getModel()->rowCount();
    int rowIdx = topLeft->row();
    int colIdx = topLeft->column();

    SqlQueryItem* item = nullptr;

    foreach (const QStringList& cells, deserializedRows)
    {
        // Check if we're out of rows range
        if (rowIdx >= rowCount)
        {
            // No more rows available.
            qDebug() << "Tried to paste more rows than available in the grid.";
            break;
        }

        foreach (const QString& cell, cells)
        {
            // Get current cell
            if (colIdx >= columnCount)
            {
                // No more columns available.
                qDebug() << "Tried to paste more columns than available in the grid.";
                break;
            }
            item = getModel()->itemFromIndex(rowIdx, colIdx);

            // Set value to the cell
            item->setValue(cell, false, false);

            // Go to next cell
            colIdx++;
        }

        // Go to next row, first cell
        rowIdx++;
        colIdx = topLeft->column();
    }
}

void SqlQueryView::copyAs()
{
    // TODO copyAs()
}

void SqlQueryView::pasteAs()
{
    // TODO pasteAs()
}

void SqlQueryView::setNull()
{
    foreach (SqlQueryItem* selItem, getSelectedItems())
        selItem->setValue(QVariant(QString::null), false, false);
}

void SqlQueryView::erase()
{
    foreach (SqlQueryItem* selItem, getSelectedItems())
        selItem->setValue("", false, false);
}

void SqlQueryView::commit()
{
    getModel()->commit();
}

void SqlQueryView::rollback()
{
    getModel()->rollback();
}

void SqlQueryView::selectiveCommit()
{
    getModel()->commit(getSelectedItems());
}

void SqlQueryView::selectiveRollback()
{
    getModel()->rollback(getSelectedItems());
}

void SqlQueryView::openValueEditor(SqlQueryItem* item)
{
    if (!item)
    {
        qWarning() << "Tried to open value editor while there's no current item. It should not be called in that case.";
        return;
    }

    MultiEditorDialog editor(this);
    editor.setWindowTitle(tr("Edit value"));
    editor.setDataType(item->getColumn()->dataType);
    editor.setValue(item->getFullValue());
    editor.setReadOnly(!item->getColumn()->canEdit());
    if (editor.exec() == QDialog::Rejected)
        return;

    item->setValue(editor.getValue());
}

void SqlQueryView::openValueEditor()
{
    SqlQueryItem* currentItem = getCurrentItem();
    openValueEditor(currentItem);
}

int qHash(SqlQueryView::Action action)
{
    return static_cast<int>(action);
}
