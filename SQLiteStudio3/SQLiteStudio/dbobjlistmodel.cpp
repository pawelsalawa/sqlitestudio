#include "dbobjlistmodel.h"
#include "db/db.h"
#include <QDebug>
#include <schemaresolver.h>

DbObjListModel::DbObjListModel(QObject *parent) :
    QAbstractListModel(parent)
{
}

QVariant DbObjListModel::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || index.row() >= objectList.size())
        return QVariant();

    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        if (sortMode == SortMode::Alphabetical)
            return objectList[index.row()];
        else
            return unsortedObjectList[index.row()];
    }

    return QVariant();
}

int DbObjListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;

    return objectList.count();
}

QModelIndex DbObjListModel::sibling(int row, int column, const QModelIndex& idx) const
{
    if (!idx.isValid() || column != 0 || row >= objectList.count())
        return QModelIndex();

    return createIndex(row, 0);
}

Db* DbObjListModel::getDb() const
{
    return db;
}

void DbObjListModel::setDb(Db* value)
{
    db = value;
    updateList();
}

DbObjListModel::SortMode DbObjListModel::getSortMode() const
{
    return sortMode;
}

void DbObjListModel::setSortMode(const SortMode& value)
{
    sortMode = value;
    beginResetModel();
    endResetModel();
}

DbObjListModel::ObjectType DbObjListModel::getType() const
{
    return type;
}

void DbObjListModel::setType(const ObjectType& value)
{
    type = value;
    updateList();
}

void DbObjListModel::updateList()
{
    if (!db || type == ObjectType::null)
        return;

    beginResetModel();
    SchemaResolver resolver(db);
    objectList = resolver.getObjects(typeString().toLower());
    unsortedObjectList = objectList;
    qSort(objectList);
    endResetModel();
}

QString DbObjListModel::typeString() const
{
    switch (type)
    {
        case ObjectType::TABLE:
            return "TABLE";
        case ObjectType::INDEX:
            return "INDEX";
        case ObjectType::TRIGGER:
            return "TRIGGER";
        case ObjectType::VIEW:
            return "VIEW";
        case ObjectType::null:
            break;
    }
    return QString::null;
}



