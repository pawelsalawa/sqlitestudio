#include "dbtreeview.h"
#include "dbtreemodel.h"
#include "dbtreeitemdelegate.h"
#include "mainwindow.h"
#include "services/dbmanager.h"
#include "common/unused.h"
#include <QDragMoveEvent>
#include <QMenu>
#include <QList>
#include <QMimeData>
#include <QDebug>

DbTreeView::DbTreeView(QWidget *parent) :
    QTreeView(parent)
{
    contextMenu = new QMenu(this);
    connect(this, &DbTreeView::customContextMenuRequested, this, &DbTreeView::showMenu);

    setHeaderHidden(true);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setSelectionMode(QAbstractItemView::ExtendedSelection);

    itemDelegate = new DbTreeItemDelegate();
    setItemDelegate(itemDelegate);

    initDndTypes();
}

DbTreeView::~DbTreeView()
{
    delete contextMenu;
    delete itemDelegate;
}

void DbTreeView::setDbTree(DbTree *dbTree)
{
    this->dbTree = dbTree;
}

DbTreeItem *DbTreeView::currentItem()
{
    return dynamic_cast<DbTreeItem*>(model()->itemFromIndex(currentIndex()));
}

DbTreeItem *DbTreeView::itemAt(const QPoint &pos)
{
    return dynamic_cast<DbTreeItem*>(model()->itemFromIndex(indexAt(pos)));
}

QList<DbTreeItem *> DbTreeView::selectionItems()
{
    QList<DbTreeItem*> items;
    QModelIndexList selectedIndexes = selectionModel()->selectedIndexes();
    foreach (QModelIndex modIdx, selectedIndexes)
        items += dynamic_cast<DbTreeItem*>(model()->itemFromIndex(modIdx));

    return items;
}

DbTreeModel *DbTreeView::model() const
{
    return dynamic_cast<DbTreeModel*>(QTreeView::model());
}

void DbTreeView::showMenu(const QPoint &pos)
{
    contextMenu->clear();

    DbTreeItem* itemUnderCursor = itemAt(pos);
    if (!itemUnderCursor)
        selectionModel()->clear();

    DbTreeItem* item = getItemForAction();
    dbTree->setupActionsForMenu(item, contextMenu);
    if (contextMenu->actions().size() == 0)
        return;

    dbTree->updateActionStates(item);
    contextMenu->popup(mapToGlobal(pos));
}

void DbTreeView::updateItemHidden(DbTreeItem* item)
{
    setRowHidden(item->index().row(), item->index().parent(), item->isHidden());
}

DbTreeItem *DbTreeView::getItemForAction() const
{
    QModelIndexList indexes = selectionModel()->selectedRows();
    if (indexes.size() == 0)
        return nullptr;

    QModelIndex idx = indexes[0];
    return dynamic_cast<DbTreeItem*>(model()->itemFromIndex(idx));
}

void DbTreeView::initDndTypes()
{
    allowedTypesInside[DbTreeItem::Type::DIR] << DbTreeItem::Type::DB << DbTreeItem::Type::DIR;
}

void DbTreeView::dragMoveEvent(QDragMoveEvent *event)
{
    QTreeView::dragMoveEvent(event);

    DbTreeItem* dstItem = itemAt(event->pos());

    //qDebug() << event->mimeData()->formats();
    const QMimeData* data = event->mimeData();
    if (data->formats().contains(DbTreeModel::MIMETYPE))
        dragMoveEventDbTreeItem(event, getDragItem(data), dstItem);
    else if (data->hasText())
        dragMoveEventString(event, data->text(), dstItem);
    else if (data->hasUrls())
        dragMoveEventUrls(event, data->urls(), dstItem);
}

void DbTreeView::dragMoveEventDbTreeItem(QDragMoveEvent *event, DbTreeItem *srcItem, DbTreeItem *dstItem)
{
    DbTreeItem::Type srcType = DbTreeItem::Type::ITEM_PROTOTYPE;
    DbTreeItem::Type dstType = DbTreeItem::Type::DIR; // the empty space is treated as group
    if (srcItem)
        srcType = srcItem->getType();

    // Depending on where we drop we need a type of item we drop ON,
    // or type of parent item if we drop ABOVE/BELOW. If we drop on empty space,
    // we leave type as default.
    if (dstItem)
    {
        QAbstractItemView::DropIndicatorPosition dropPosition = dropIndicatorPosition();
        switch (dropPosition)
        {
            case QAbstractItemView::OnItem:
                dstType = dynamic_cast<DbTreeItem*>(dstItem)->getType();
                break;
            case QAbstractItemView::AboveItem:
            case QAbstractItemView::BelowItem:
            {
                QStandardItem* parentItem = dstItem->parentItem();
                if (dynamic_cast<DbTreeItem*>(parentItem))
                    dstType = dynamic_cast<DbTreeItem*>(parentItem)->getType();
                break;
            }
            case QAbstractItemView::OnViewport:
                break;
        }
    }

    if (!allowedTypesInside[dstType].contains(srcType))
        event->ignore();
}

void DbTreeView::dragMoveEventString(QDragMoveEvent *event, const QString &srcString, DbTreeItem *dstItem)
{
    // TODO
}

void DbTreeView::dragMoveEventUrls(QDragMoveEvent *event, const QList<QUrl> &srcUrls, DbTreeItem *dstItem)
{
    // TODO
}

DbTreeItem *DbTreeView::getDragItem(const QMimeData* data) const
{
    QByteArray byteData = data->data(DbTreeModel::MIMETYPE);
    QDataStream stream(&byteData, QIODevice::ReadOnly);
    quint64 itemAddr;
    stream >> itemAddr;
    return dynamic_cast<DbTreeItem*>(reinterpret_cast<QStandardItem*>(itemAddr));
}

void DbTreeView::mouseDoubleClickEvent(QMouseEvent *event)
{
    DbTreeItem* itemUnderCursor = itemAt(event->pos());
    if (itemUnderCursor && !handleDoubleClick(itemUnderCursor))
        return;

    QTreeView::mouseDoubleClickEvent(event);
}

bool DbTreeView::handleDoubleClick(DbTreeItem *item)
{
    switch (item->getType())
    {
        case DbTreeItem::Type::DIR:
            break;
        case DbTreeItem::Type::DB:
            return handleDbDoubleClick(item);
        case DbTreeItem::Type::INVALID_DB:
            break;
        case DbTreeItem::Type::TABLES:
            break;
        case DbTreeItem::Type::VIRTUAL_TABLE:
            // TODO if module for virtual table is loaded - show virtual table window
            break;
        case DbTreeItem::Type::TABLE:
            return handleTableDoubleClick(item);
        case DbTreeItem::Type::INDEXES:
            break;
        case DbTreeItem::Type::INDEX:
            return handleIndexDoubleClick(item);
        case DbTreeItem::Type::TRIGGERS:
            break;
        case DbTreeItem::Type::TRIGGER:
            return handleTriggerDoubleClick(item);
        case DbTreeItem::Type::VIEWS:
            break;
        case DbTreeItem::Type::VIEW:
            return handleViewDoubleClick(item);
        case DbTreeItem::Type::COLUMNS:
            break;
        case DbTreeItem::Type::COLUMN:
            return handleColumnDoubleClick(item);
        case DbTreeItem::Type::ITEM_PROTOTYPE:
            break;
    }

    return true;
}

bool DbTreeView::handleDbDoubleClick(DbTreeItem *item)
{
    if (!item->getDb()->isOpen())
    {
        dbTree->getAction(DbTree::CONNECT_TO_DB)->trigger();
        return false;
    }
    return true;
}

bool DbTreeView::handleTableDoubleClick(DbTreeItem *item)
{
    dbTree->openTable(item);
    return false;
}

bool DbTreeView::handleIndexDoubleClick(DbTreeItem *item)
{
    dbTree->editIndex(item);
    return false;
}

bool DbTreeView::handleTriggerDoubleClick(DbTreeItem *item)
{
    dbTree->editTrigger(item);
    return false;
}

bool DbTreeView::handleViewDoubleClick(DbTreeItem *item)
{
    dbTree->openView(item);
    return false;
}

bool DbTreeView::handleColumnDoubleClick(DbTreeItem *item)
{
    dbTree->editColumn(item);
    return false;
}
