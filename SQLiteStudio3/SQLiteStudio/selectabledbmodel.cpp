#include "selectabledbmodel.h"
#include "dbtree/dbtreeitem.h"
#include "dbtree/dbtreemodel.h"

SelectableDbModel::SelectableDbModel(QObject* parent) :
    QSortFilterProxyModel(parent)
{
}

QVariant SelectableDbModel::data(const QModelIndex& index, int role) const
{
    if (role != Qt::CheckStateRole)
        return QSortFilterProxyModel::data(index, role);

    DbTreeItem* item = getItemForProxyIndex(index);
    if (!item)
        return QSortFilterProxyModel::data(index, role);

    DbTreeItem::Type type = item->getType();
    if (type != DbTreeItem::Type::DB && type != DbTreeItem::Type::INVALID_DB)
        return QSortFilterProxyModel::data(index, role);

    return checkedDatabases.contains(item->text(), Qt::CaseInsensitive) ? Qt::Checked : Qt::Unchecked;
}

bool SelectableDbModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (role != Qt::CheckStateRole)
        return QSortFilterProxyModel::setData(index, value, role);

    DbTreeItem* item = getItemForProxyIndex(index);
    if (!item)
        return QSortFilterProxyModel::setData(index, value, role);

    DbTreeItem::Type type = item->getType();
    if (type != DbTreeItem::Type::DB && type != DbTreeItem::Type::INVALID_DB)
        return QSortFilterProxyModel::setData(index, value, role);

    if (value.toBool())
        checkedDatabases << item->text();
    else
        checkedDatabases.removeOne(item->text());

    emit dataChanged(index, index, {Qt::CheckStateRole});

    return true;
}

Qt::ItemFlags SelectableDbModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags itemFlags = QSortFilterProxyModel::flags(index);

    DbTreeItem* item = getItemForProxyIndex(index);
    if (!item)
        return itemFlags;

    DbTreeItem::Type type = item->getType();
    if (item->getDb() && item->getDb()->getVersion() == disabledVersion)
        itemFlags ^= Qt::ItemIsEnabled;
    else if (type == DbTreeItem::Type::DB || type == DbTreeItem::Type::INVALID_DB)
        itemFlags |= Qt::ItemIsUserCheckable;

    return itemFlags;
}

void SelectableDbModel::setDatabases(const QStringList& databases)
{
    beginResetModel();
    checkedDatabases = databases;
    endResetModel();
}

QStringList SelectableDbModel::getDatabases() const
{
    return checkedDatabases;
}

bool SelectableDbModel::filterAcceptsRow(int srcRow, const QModelIndex& srcParent) const
{
    QModelIndex idx = sourceModel()->index(srcRow, 0, srcParent);
    DbTreeItem* item = dynamic_cast<DbTreeItem*>(dynamic_cast<DbTreeModel*>(sourceModel())->itemFromIndex(idx));
    switch (item->getType())
    {
        case DbTreeItem::Type::DIR:
        case DbTreeItem::Type::DB:
        case DbTreeItem::Type::INVALID_DB:
            return true;
        default:
            return false;
    }
    return false;
}

DbTreeItem* SelectableDbModel::getItemForProxyIndex(const QModelIndex& index) const
{
    QModelIndex srcIdx = mapToSource(index);
    DbTreeItem* item = dynamic_cast<DbTreeItem*>(dynamic_cast<DbTreeModel*>(sourceModel())->itemFromIndex(srcIdx));
    return item;
}
int SelectableDbModel::getDisabledVersion() const
{
    return disabledVersion;
}

void SelectableDbModel::setDisabledVersion(int value)
{
    disabledVersion = value;
}


