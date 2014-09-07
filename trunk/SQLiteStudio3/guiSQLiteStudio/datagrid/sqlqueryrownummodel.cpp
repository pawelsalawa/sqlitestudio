#include "sqlqueryrownummodel.h"
#include "common/unused.h"

SqlQueryRowNumModel::SqlQueryRowNumModel(QAbstractItemModel *value, QObject *parent) :
    QAbstractItemModel(parent)
{
    mainModel = value;
}

QModelIndex SqlQueryRowNumModel::index(int row, int column, const QModelIndex &parent) const
{
    UNUSED(row);
    UNUSED(column);
    UNUSED(parent);
    return QModelIndex();
}

QModelIndex SqlQueryRowNumModel::parent(const QModelIndex &child) const
{
    UNUSED(child);
    return QModelIndex();
}

int SqlQueryRowNumModel::rowCount(const QModelIndex &parent) const
{
    UNUSED(parent);

    if (!mainModel)
        return 0;

    return mainModel->rowCount();
}

int SqlQueryRowNumModel::columnCount(const QModelIndex &parent) const
{
    UNUSED(parent);

    if (!mainModel)
        return 0;

    return mainModel->columnCount();
}

QVariant SqlQueryRowNumModel::data(const QModelIndex &index, int role) const
{
    UNUSED(index);
    UNUSED(role);
    return QVariant();
}

QVariant SqlQueryRowNumModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    UNUSED(role);
    if (orientation == Qt::Horizontal)
        return QVariant();

    return rowNumBase + section;
}

void SqlQueryRowNumModel::setRowNumBase(int value)
{
    rowNumBase = value;
}
