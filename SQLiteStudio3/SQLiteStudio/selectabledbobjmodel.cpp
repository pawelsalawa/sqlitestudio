#include "selectabledbobjmodel.h"
#include "dbtree/dbtreeitem.h"
#include "dbtree/dbtreemodel.h"

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

Qt::ItemFlags SelectableDbObjModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags flags = QSortFilterProxyModel::flags(index);
    DbTreeItem* item = getItemForProxyIndex(index);
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
            if (index.child(0, 0).isValid())
                flags |= Qt::ItemIsTristate;

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
    return checkedObjects.toList();
}

void SelectableDbObjModel::setCheckedObjects(const QStringList& value)
{
    checkedObjects = value.toSet();
}

void SelectableDbObjModel::setRootChecked(bool checked)
{
    QModelIndex idx = index(0, 0);
    if (!idx.isValid())
        return;

    setData(idx, checked ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole);
}

bool SelectableDbObjModel::filterAcceptsRow(int srcRow, const QModelIndex& srcParent) const
{
    QModelIndex idx = sourceModel()->index(srcRow, 0, srcParent);
    DbTreeItem* item = dynamic_cast<DbTreeItem*>(dynamic_cast<DbTreeModel*>(sourceModel())->itemFromIndex(idx));
    DbTreeItem* dbItem = item->getPathToParentItem(DbTreeItem::Type::DB).last();

    if (!dbItem || !dbItem->getDb() || dbItem->getDb()->getName() != dbName)
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

Qt::CheckState SelectableDbObjModel::getStateFromChilds(const QModelIndex& index) const
{
    DbTreeItem* item = getItemForProxyIndex(index);
    if (!item)
        return Qt::Unchecked;

    if (!index.child(0, 0).isValid())
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
    for (int i = 0; (child = index.child(i, 0)).isValid(); i++)
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

void SelectableDbObjModel::setRecurrently(const QModelIndex& index, Qt::CheckState checked)
{
    DbTreeItem* item = getItemForProxyIndex(index);
    if (!item)
        return;

    if (checked && isObject(item))
        checkedObjects << item->text();
    else
        checkedObjects.remove(item->text());

    if (!index.child(0, 0).isValid())
        return;

    // Limiting checked to 'checked/unchecked', cause recurrent marking cannot set partially checked, it makes no sense
    checked = (checked > 0 ? Qt::Checked : Qt::Unchecked);

    QModelIndex child;
    for (int i = 0; (child = index.child(i, 0)).isValid(); i++)
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



