#include "dbtreeview.h"
#include "dbtreemodel.h"
#include "actionentry.h"
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

void DbTreeView::setupActionsForMenu(DbTreeItem *currItem)
{
    QList<ActionEntry> actions;

    ActionEntry dbEntry(ICONS.DATABASE, tr("Datatabase"));
    dbEntry += DbTree::ADD_DB;
    dbEntry += DbTree::EDIT_DB;
    dbEntry += DbTree::DELETE_DB;

    ActionEntry dbEntryExt(ICONS.DATABASE, tr("Datatabase"));
    dbEntryExt += DbTree::CONNECT_TO_DB;
    dbEntryExt += DbTree::DISCONNECT_FROM_DB;
    dbEntryExt += DbTree::_separator;
    dbEntryExt += DbTree::ADD_DB;
    dbEntryExt += DbTree::EDIT_DB;
    dbEntryExt += DbTree::DELETE_DB;
    dbEntryExt += DbTree::_separator;
    dbEntryExt += DbTree::REFRESH_SCHEMA;

    ActionEntry groupEntry(ICONS.DIRECTORY, tr("Grouping"));
    groupEntry += DbTree::CREATE_GROUP;
    groupEntry += DbTree::RENAME_GROUP;
    groupEntry += DbTree::DELETE_GROUP;

    if (currItem)
    {
        // Parent item gives context for TRIGGERS type
        DbTreeItem* parentItem = currItem->parentDbTreeItem();

        DbTreeItem::Type itemType = currItem->getType();
        switch (itemType)
        {
            case DbTreeItem::Type::DIR:
            {
                actions += ActionEntry(DbTree::CREATE_GROUP);
                actions += ActionEntry(DbTree::RENAME_GROUP);
                actions += ActionEntry(DbTree::DELETE_GROUP);
                actions += ActionEntry(DbTree::_separator);
                actions += dbEntry;
                break;
            }
            case DbTreeItem::Type::DB:
            {
                actions += ActionEntry(DbTree::CONNECT_TO_DB);
                actions += ActionEntry(DbTree::DISCONNECT_FROM_DB);
                actions += ActionEntry(DbTree::_separator);
                actions += ActionEntry(DbTree::ADD_DB);
                actions += ActionEntry(DbTree::EDIT_DB);
                actions += ActionEntry(DbTree::DELETE_DB);
                actions += ActionEntry(DbTree::_separator);
                actions += ActionEntry(DbTree::REFRESH_SCHEMA);
                actions += ActionEntry(DbTree::_separator);
                actions += ActionEntry(DbTree::ADD_TABLE);
                actions += ActionEntry(DbTree::ADD_INDEX);
                actions += ActionEntry(DbTree::ADD_TRIGGER);
                actions += ActionEntry(DbTree::ADD_VIEW);
                actions += ActionEntry(DbTree::_separator);
                break;
            }
            case DbTreeItem::Type::INVALID_DB:
            {
                actions += ActionEntry(DbTree::ADD_DB);
                actions += ActionEntry(DbTree::EDIT_DB);
                actions += ActionEntry(DbTree::DELETE_DB);
                actions += ActionEntry(DbTree::_separator);
                break;
            }
            case DbTreeItem::Type::TABLES:
                actions += ActionEntry(DbTree::ADD_TABLE);
                actions += ActionEntry(DbTree::EDIT_TABLE);
                actions += ActionEntry(DbTree::DEL_TABLE);
                actions += ActionEntry(DbTree::_separator);
                actions += dbEntryExt;
                break;
            case DbTreeItem::Type::TABLE:
                actions += ActionEntry(DbTree::ADD_TABLE);
                actions += ActionEntry(DbTree::EDIT_TABLE);
                actions += ActionEntry(DbTree::DEL_TABLE);
                actions += ActionEntry(DbTree::_separator);
                actions += ActionEntry(DbTree::ADD_INDEX);
                actions += ActionEntry(DbTree::EDIT_INDEX);
                actions += ActionEntry(DbTree::DEL_INDEX);
                actions += ActionEntry(DbTree::_separator);
                actions += ActionEntry(DbTree::ADD_TRIGGER);
                actions += ActionEntry(DbTree::EDIT_TRIGGER);
                actions += ActionEntry(DbTree::DEL_TRIGGER);
                actions += ActionEntry(DbTree::_separator);
                actions += dbEntryExt;
                break;
            case DbTreeItem::Type::INDEXES:
                actions += ActionEntry(DbTree::ADD_TABLE);
                actions += ActionEntry(DbTree::EDIT_TABLE);
                actions += ActionEntry(DbTree::DEL_TABLE);
                actions += ActionEntry(DbTree::_separator);
                actions += ActionEntry(DbTree::ADD_INDEX);
                actions += ActionEntry(DbTree::EDIT_INDEX);
                actions += ActionEntry(DbTree::DEL_INDEX);
                actions += ActionEntry(DbTree::_separator);
                actions += dbEntryExt;
                break;
            case DbTreeItem::Type::INDEX:
                actions += ActionEntry(DbTree::ADD_TABLE);
                actions += ActionEntry(DbTree::EDIT_TABLE);
                actions += ActionEntry(DbTree::DEL_TABLE);
                actions += ActionEntry(DbTree::_separator);
                actions += ActionEntry(DbTree::ADD_INDEX);
                actions += ActionEntry(DbTree::EDIT_INDEX);
                actions += ActionEntry(DbTree::DEL_INDEX);
                actions += ActionEntry(DbTree::_separator);
                actions += dbEntryExt;
                break;
            case DbTreeItem::Type::TRIGGERS:
            {
                if (parentItem->getType() == DbTreeItem::Type::TABLE)
                {
                    actions += ActionEntry(DbTree::ADD_TABLE);
                    actions += ActionEntry(DbTree::EDIT_TABLE);
                    actions += ActionEntry(DbTree::DEL_TABLE);
                }
                else
                {
                    actions += ActionEntry(DbTree::ADD_VIEW);
                    actions += ActionEntry(DbTree::EDIT_VIEW);
                    actions += ActionEntry(DbTree::DEL_VIEW);
                }
                actions += ActionEntry(DbTree::_separator);
                actions += ActionEntry(DbTree::ADD_TRIGGER);
                actions += ActionEntry(DbTree::EDIT_TRIGGER);
                actions += ActionEntry(DbTree::DEL_TRIGGER);
                actions += ActionEntry(DbTree::_separator);
                actions += dbEntryExt;
                break;
            }
            case DbTreeItem::Type::TRIGGER:
            {
                if (parentItem->getType() == DbTreeItem::Type::TABLE)
                {
                    actions += ActionEntry(DbTree::ADD_TABLE);
                    actions += ActionEntry(DbTree::EDIT_TABLE);
                    actions += ActionEntry(DbTree::DEL_TABLE);
                }
                else
                {
                    actions += ActionEntry(DbTree::ADD_VIEW);
                    actions += ActionEntry(DbTree::EDIT_VIEW);
                    actions += ActionEntry(DbTree::DEL_VIEW);
                }
                actions += ActionEntry(DbTree::_separator);
                actions += ActionEntry(DbTree::ADD_TRIGGER);
                actions += ActionEntry(DbTree::EDIT_TRIGGER);
                actions += ActionEntry(DbTree::DEL_TRIGGER);
                actions += ActionEntry(DbTree::_separator);
                actions += dbEntryExt;
                break;
            }
            case DbTreeItem::Type::VIEWS:
                actions += ActionEntry(DbTree::ADD_VIEW);
                actions += ActionEntry(DbTree::EDIT_VIEW);
                actions += ActionEntry(DbTree::DEL_VIEW);
                actions += ActionEntry(DbTree::_separator);
                actions += dbEntryExt;
                break;
            case DbTreeItem::Type::VIEW:
                actions += ActionEntry(DbTree::ADD_VIEW);
                actions += ActionEntry(DbTree::EDIT_VIEW);
                actions += ActionEntry(DbTree::DEL_VIEW);
                actions += ActionEntry(DbTree::_separator);
                actions += ActionEntry(DbTree::ADD_TRIGGER);
                actions += ActionEntry(DbTree::EDIT_TRIGGER);
                actions += ActionEntry(DbTree::DEL_TRIGGER);
                actions += ActionEntry(DbTree::_separator);
                actions += dbEntryExt;
                break;
            case DbTreeItem::Type::COLUMNS:
                actions += ActionEntry(DbTree::ADD_TABLE);
                actions += ActionEntry(DbTree::EDIT_TABLE);
                actions += ActionEntry(DbTree::DEL_TABLE);
                actions += ActionEntry(DbTree::_separator);
                actions += dbEntryExt;
                break;
            case DbTreeItem::Type::COLUMN:
                actions += ActionEntry(DbTree::EDIT_COLUMN);
                actions += ActionEntry(DbTree::_separator);
                actions += ActionEntry(DbTree::ADD_TABLE);
                actions += ActionEntry(DbTree::EDIT_TABLE);
                actions += ActionEntry(DbTree::DEL_TABLE);
                actions += ActionEntry(DbTree::_separator);
                actions += dbEntryExt;
                break;
            case DbTreeItem::Type::ITEM_PROTOTYPE:
                break;
        }

        actions += ActionEntry(DbTree::REFRESH_SCHEMAS);
        actions += ActionEntry(DbTree::_separator);

        if (itemType == DbTreeItem::Type::DB)
            actions += groupEntry;
    }
    else
    {
        actions += dbEntry;
        actions += ActionEntry(DbTree::REFRESH_SCHEMAS);
        actions += ActionEntry(DbTree::_separator);
        actions += groupEntry;
    }

    actions += DbTree::COPY;
    actions += DbTree::PASTE;
    actions += DbTree::_separator;
    actions += DbTree::SELECT_ALL;

    QMenu* subMenu;
    foreach (ActionEntry actionEntry, actions)
    {
        switch (actionEntry.type)
        {
            case ActionEntry::Type::SINGLE:
            {
                if (actionEntry.action == DbTree::_separator)
                {
                    contextMenu->addSeparator();
                    break;
                }
                contextMenu->addAction(dbTree->getAction(actionEntry.action));
                break;
            }
            case ActionEntry::Type::SUB_MENU:
            {
                subMenu = contextMenu->addMenu(actionEntry.subMenuIcon, actionEntry.subMenuLabel);
                foreach (DbTree::Action action, actionEntry.actions)
                {
                    if (action == DbTree::_separator)
                    {
                        subMenu->addSeparator();
                        continue;
                    }
                    subMenu->addAction(dbTree->getAction(action));
                }
                break;
            }
        }
    }
}

void DbTreeView::showMenu(const QPoint &pos)
{
    contextMenu->clear();

    DbTreeItem* itemUnderCursor = itemAt(pos);
    if (!itemUnderCursor)
        selectionModel()->clear();

    DbTreeItem* item = getItemForAction();
    setupActionsForMenu(item);
    if (contextMenu->actions().size() == 0)
        return;

    dbTree->updateActionsFor(item);
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
