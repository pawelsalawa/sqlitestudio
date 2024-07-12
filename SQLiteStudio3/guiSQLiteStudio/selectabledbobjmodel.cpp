#include "selectabledbobjmodel.h"
#include "dbtree/dbtreeitem.h"
#include "dbtree/dbtreemodel.h"
#include "common/compatibility.h"
#include <QDebug>
#include <QTreeView>

SelectableDbObjModel::SelectableDbObjModel(QObject *parent) :
    QSortFilterProxyModel(parent)
{
}

QVariant SelectableDbObjModel::data(const QModelIndex& index, int role) const
{
    if (role != Qt::CheckStateRole)
        return QSortFilterProxyModel::data(index, role);

    return getStateFromChilds(index);
}

bool SelectableDbObjModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (role != Qt::CheckStateRole)
        return QSortFilterProxyModel::setData(index, value, role);

    Qt::CheckState checked = static_cast<Qt::CheckState>(value.toInt());
    setRecurrently(index, checked);
    emit dataChanged(index, index, {Qt::CheckStateRole});

    return true;
}

Qt::ItemFlags SelectableDbObjModel::flags(const QModelIndex& idx) const
{
    Qt::ItemFlags flags = QSortFilterProxyModel::flags(idx);
    DbTreeItem* item = getItemForProxyIndex(idx);
    switch (item->getType())
    {
        case DbTreeItem::Type::TABLE:
        case DbTreeItem::Type::VIRTUAL_TABLE:
        case DbTreeItem::Type::INDEX:
        case DbTreeItem::Type::TRIGGER:
        case DbTreeItem::Type::VIEW:
        case DbTreeItem::Type::DB:
        case DbTreeItem::Type::TABLES:
        case DbTreeItem::Type::INDEXES:
        case DbTreeItem::Type::TRIGGERS:
        case DbTreeItem::Type::VIEWS:
        {
            flags |= Qt::ItemIsUserCheckable;
            if (index(0, 0, idx).isValid())
                flags |= Qt::ItemIsAutoTristate;

            break;
        }
        case DbTreeItem::Type::DIR:
        case DbTreeItem::Type::COLUMNS:
        case DbTreeItem::Type::COLUMN:
        case DbTreeItem::Type::ITEM_PROTOTYPE:
            break;
    }

    return flags;
}

QString SelectableDbObjModel::getDbName() const
{
    return dbName;
}

void SelectableDbObjModel::setDbName(const QString& value)
{
    beginResetModel();
    dbName = value;
    checkedObjects.clear();
    endResetModel();
}
QStringList SelectableDbObjModel::getCheckedObjects() const
{
    return checkedObjects.values();
}

void SelectableDbObjModel::setCheckedObjects(const QStringList& value)
{
    checkedObjects = toSet(value);
}

void SelectableDbObjModel::setRootChecked(bool checked)
{
    QModelIndex idx = index(0, 0);
    if (!idx.isValid())
        return;

    setData(idx, checked ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole);
}

DbTreeItem* SelectableDbObjModel::getItemForIndex(const QModelIndex& index) const
{
    return getItemForProxyIndex(index);
}

bool SelectableDbObjModel::filterAcceptsRow(int srcRow, const QModelIndex& srcParent) const
{
    QModelIndex idx = sourceModel()->index(srcRow, 0, srcParent);
    DbTreeItem* item = dynamic_cast<DbTreeItem*>(dynamic_cast<DbTreeModel*>(sourceModel())->itemFromIndex(idx));
    DbTreeItem* dbItem = item->getPathToParentItem(DbTreeItem::Type::DB).last();

    // These 3 conditions could be written as one OR-ed, but this is easier to debug which one fails this way.
    if (!dbItem)
        return false;

    if (item->getType() == DbTreeItem::Type::DIR)
        return checkRecurrentlyForDb(item);

    if (!dbItem->getDb())
        return false;

    if (dbItem->getDb()->getName() != dbName)
        return false;

    switch (item->getType())
    {
        case DbTreeItem::Type::TABLE:
        case DbTreeItem::Type::VIRTUAL_TABLE:
        case DbTreeItem::Type::INDEX:
        case DbTreeItem::Type::TRIGGER:
        case DbTreeItem::Type::VIEW:
        case DbTreeItem::Type::DB:
            return true;
        case DbTreeItem::Type::TABLES:
        case DbTreeItem::Type::INDEXES:
        case DbTreeItem::Type::TRIGGERS:
        case DbTreeItem::Type::VIEWS:
            return item->rowCount() > 0;
        case DbTreeItem::Type::DIR:
        case DbTreeItem::Type::COLUMNS:
        case DbTreeItem::Type::COLUMN:
        case DbTreeItem::Type::ITEM_PROTOTYPE:
            return false;
    }
    return false;
}

DbTreeItem* SelectableDbObjModel::getItemForProxyIndex(const QModelIndex& index) const
{
    QModelIndex srcIdx = mapToSource(index);
    DbTreeItem* item = dynamic_cast<DbTreeItem*>(dynamic_cast<DbTreeModel*>(sourceModel())->itemFromIndex(srcIdx));
    return item;
}

Qt::CheckState SelectableDbObjModel::getStateFromChilds(const QModelIndex& idx) const
{
    DbTreeItem* item = getItemForProxyIndex(idx);
    if (!item)
        return Qt::Unchecked;

    if (!index(0, 0, idx).isValid())
    {
        if (isObject(item) && checkedObjects.contains(item->text()))
            return Qt::Checked;
        else
            return Qt::Unchecked;
    }

    int total = 0;
    int checked = 0;
    int partial = 0;
    Qt::CheckState state;
    QModelIndex child;
    for (int i = 0; (child = index(i, 0, idx)).isValid(); i++)
    {
        if (!child.flags().testFlag(Qt::ItemIsUserCheckable))
            continue;

        total++;
        state = static_cast<Qt::CheckState>(child.data(Qt::CheckStateRole).toInt());
        if (state == Qt::Checked || state == Qt::PartiallyChecked)
        {
            checked++;
            if (state == Qt::PartiallyChecked)
                partial++;
        }
    }

    if (total == checked)
    {
        if (partial > 0)
            return Qt::PartiallyChecked;
        else
            return Qt::Checked;
    }

    if (checked == 0)
    {
        if (isObject(item) && checkedObjects.contains(item->text()))
            return Qt::PartiallyChecked;
        else
            return Qt::Unchecked;
    }

    return Qt::PartiallyChecked;
}

void SelectableDbObjModel::setRecurrently(const QModelIndex& idx, Qt::CheckState checked)
{
    DbTreeItem* item = getItemForProxyIndex(idx);
    if (!item)
        return;

    if (checked && isObject(item))
        checkedObjects << item->text();
    else
        checkedObjects.remove(item->text());

    if (!index(0, 0, idx).isValid())
        return;

    // Limiting checked to 'checked/unchecked', cause recurrent marking cannot set partially checked, it makes no sense
    checked = (checked > 0 ? Qt::Checked : Qt::Unchecked);

    QModelIndex child;
    for (int i = 0; (child = index(i, 0, idx)).isValid(); i++)
        setData(child, checked, Qt::CheckStateRole);
}

bool SelectableDbObjModel::isObject(DbTreeItem* item) const
{
    switch (item->getType())
    {
        case DbTreeItem::Type::TABLE:
        case DbTreeItem::Type::INDEX:
        case DbTreeItem::Type::TRIGGER:
        case DbTreeItem::Type::VIEW:
        case DbTreeItem::Type::VIRTUAL_TABLE:
            return true;
        default:
            break;
    }
    return false;
}

bool SelectableDbObjModel::checkRecurrentlyForDb(DbTreeItem* item) const
{
    return item->findItem(DbTreeItem::Type::DB, dbName) != nullptr;
}
