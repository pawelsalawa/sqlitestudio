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
#include "services/notifymanager.h"
#include "windows/editorwindow.h"
#include "mainwindow.h"
#include "common/utils_sql.h"
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
#include <QMimeData>
#include <QCryptographicHash>

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

void SqlQueryView::init()
{
    itemDelegate = new SqlQueryItemDelegate();
    setItemDelegate(itemDelegate);
    setMouseTracking(true);
    setEditTriggers(QAbstractItemView::AnyKeyPressed);

    setContextMenuPolicy(Qt::CustomContextMenu);
    contextMenu = new QMenu(this);
    referencedTablesMenu = new QMenu(tr("Go to referenced row in..."), contextMenu);

    connect(this, &QWidget::customContextMenuRequested, this, &SqlQueryView::customContextMenuRequested);
    connect(CFG_UI.Fonts.DataView, SIGNAL(changed(QVariant)), this, SLOT(updateFont()));
    connect(this, SIGNAL(activated(QModelIndex)), this, SLOT(itemActivated(QModelIndex)));

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

    if (selectedItems.size() == 1 && selectedItems.first() == currentItem)
        addFkActionsToContextMenu(currentItem);

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
    SqlQueryModel* m = getModel();
    connect(widgetCover, SIGNAL(cancelClicked()), m, SLOT(interrupt()));
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

void SqlQueryView::itemActivated(const QModelIndex& index)
{
    if (!index.isValid())
        return;

    SqlQueryItem* item = getModel()->itemFromIndex(index);
    if (!item)
        return;

    if (!editInEditorIfNecessary(item))
        return;

    edit(getCurrentIndex());
}

bool SqlQueryView::editInEditorIfNecessary(SqlQueryItem* item)
{
    if (item->getColumn()->dataType.getType() == DataType::BLOB)
    {
        openValueEditor(item);
        return false;
    }
    return true;
}

void SqlQueryView::paste(const QList<QList<QVariant> >& data)
{
    QList<SqlQueryItem*> selectedItems = getSelectedItems();
    if (selectedItems.isEmpty())
    {
        notifyWarn(tr("No items selected to paste clipboard contents to."));
        return;
    }

    qSort(selectedItems);
    SqlQueryItem* topLeft = selectedItems.first();

    int columnCount = getModel()->columnCount();
    int rowCount = getModel()->rowCount();
    int rowIdx = topLeft->row();
    int colIdx = topLeft->column();

    SqlQueryItem* item = nullptr;

    foreach (const QList<QVariant>& cells, data)
    {
        // Check if we're out of rows range
        if (rowIdx >= rowCount)
        {
            // No more rows available.
            qDebug() << "Tried to paste more rows than available in the grid.";
            break;
        }

        foreach (const QVariant& cell, cells)
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

void SqlQueryView::addFkActionsToContextMenu(SqlQueryItem* currentItem)
{
    QList<SqlQueryModelColumn::ConstraintFk*> fkList = currentItem->getColumn()->getFkConstraints();
    if (fkList.isEmpty())
        return;

    QAction* act;
    if (fkList.size() == 1)
    {
        SqlQueryModelColumn::ConstraintFk* fk = fkList.first();
        act = contextMenu->addAction(tr("Go to referenced row in table '%1'").arg(fk->foreignTable));
        connect(act, &QAction::triggered, [this, fk, currentItem](bool) {
            goToReferencedRow(fk->foreignTable, fk->foreignColumn, currentItem->getValue());
        });
        contextMenu->addSeparator();
        return;
    }

    referencedTablesMenu->clear();
    contextMenu->addMenu(referencedTablesMenu);
    for (SqlQueryModelColumn::ConstraintFk* fk : fkList)
    {
        act = referencedTablesMenu->addAction(tr("table '%1'").arg(fk->foreignTable));
        connect(act, &QAction::triggered, [this, fk, currentItem](bool) {
            goToReferencedRow(fk->foreignTable, fk->foreignColumn, currentItem->getValue());
        });
    }
    contextMenu->addSeparator();
}

void SqlQueryView::goToReferencedRow(const QString& table, const QString& column, const QVariant& value)
{
    Db* db = getModel()->getDb();
    if (!db || !db->isValid())
        return;

    EditorWindow* win = MAINWINDOW->openSqlEditor();
    if (!win->setCurrentDb(db))
    {
        qCritical() << "Created EditorWindow had not got requested database:" << db->getName();
        win->close();
        return;
    }

    static QString sql = QStringLiteral("SELECT * FROM %1 WHERE %2 = %3");

    QString valueStr = wrapValueIfNeeded(value.toString());

    win->getMdiWindow()->rename(tr("Referenced row (%1)").arg(table));
    win->setContents(sql.arg(table, column, valueStr));
    win->execute();
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

    QVariant itemValue;
    QStringList cells;
    QList<QStringList> rows;

    QPair<QString,QList<QList<QVariant>>> theDataPair;
    QList<QList<QVariant>> theData;
    QList<QVariant> theDataRow;

    foreach (const QList<SqlQueryItem*>& itemsInRows, groupedItems)
    {
        foreach (SqlQueryItem* item, itemsInRows)
        {
            itemValue = item->getFullValue();
            cells << itemValue.toString();
            theDataRow << itemValue;
        }

        rows << cells;
        cells.clear();

        theData << theDataRow;
        theDataRow.clear();
    }

    QMimeData* mimeData = new QMimeData();
    QString tsv = TsvSerializer::serialize(rows);
    mimeData->setText(tsv);

    QString md5 = QCryptographicHash::hash(tsv.toUtf8(), QCryptographicHash::Md5);
    theDataPair.first = md5;
    theDataPair.second = theData;

    QByteArray serializedData;
    QDataStream stream(&serializedData, QIODevice::WriteOnly);
    stream << theDataPair;
    mimeData->setData(mimeDataId, serializedData);

    qApp->clipboard()->setMimeData(mimeData);
}

void SqlQueryView::paste()
{
    const QMimeData* mimeData = qApp->clipboard()->mimeData();
    if (mimeData->hasFormat(mimeDataId))
    {
        QString tsv = mimeData->text();
        QString md5 = QCryptographicHash::hash(tsv.toUtf8(), QCryptographicHash::Md5);

        QPair<QString,QList<QList<QVariant>>> theDataPair;
        QByteArray serializedData = mimeData->data(mimeDataId);
        QDataStream stream(&serializedData, QIODevice::ReadOnly);
        stream >> theDataPair;

        if (md5 == theDataPair.first)
        {
            paste(theDataPair.second);
            return;
        }
    }

    QList<QStringList> deserializedRows = TsvSerializer::deserialize(mimeData->text());

    QList<QVariant> dataRow;
    QList<QList<QVariant>> dataToPaste;
    for (const QStringList& cells : deserializedRows)
    {
        for (const QString& cell : cells)
            dataRow << cell;

        dataToPaste << dataRow;
        dataRow.clear();
    }

    paste(dataToPaste);
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
    for (SqlQueryItem* selItem : getSelectedItems()) {
        if (selItem->getColumn()->editionForbiddenReason.size() > 0)
            continue;

        selItem->setValue(QVariant(QString::null), false, false);
    }
}

void SqlQueryView::erase()
{
    for (SqlQueryItem* selItem : getSelectedItems()) {
        if (selItem->getColumn()->editionForbiddenReason.size() > 0)
            continue;

        selItem->setValue("", false, false);
    }
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
