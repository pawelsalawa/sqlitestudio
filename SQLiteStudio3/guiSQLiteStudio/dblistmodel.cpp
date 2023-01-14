#include "dblistmodel.h"
#include "services/dbmanager.h"
#include "dbtree/dbtree.h"
#include "dbtree/dbtreemodel.h"
#include "uiconfig.h"
#include <QComboBox>

DbListModel::DbListModel(QObject *parent) :
    QAbstractListModel(parent)
{
    unsortedList = DBLIST->getConnectedDbList();

    connect(DBLIST, SIGNAL(dbConnected(Db*)), this, SLOT(dbConnected(Db*)));
    connect(DBLIST, SIGNAL(dbDisconnected(Db*)), this, SLOT(dbDisconnected(Db*)));

    setSortMode(CFG_UI.General.SqlEditorDbListOrder.get());
}

DbListModel::~DbListModel()
{
}

QVariant DbListModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= dbList.size())
        return QVariant();

    if (role == Qt::DisplayRole || role == Qt::EditRole)
        return dbList[index.row()]->getName();

    return QVariant();
}

int DbListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return dbList.count();
}

QModelIndex DbListModel::sibling(int row, int column, const QModelIndex &idx) const
{
    if (!idx.isValid() || column != 0 || row >= dbList.count())
        return QModelIndex();

    return createIndex(row, 0);
}

Db* DbListModel::getDb(int index)
{
    if (index < 0 || index >= dbList.size())
        return nullptr;

    return dbList[index];
}

int DbListModel::getIndexForDb(Db* db)
{
    return dbList.indexOf(db);
}

void DbListModel::setSortMode(DbListModel::SortMode sortMode)
{
    this->sortMode = sortMode;
    sort();
}

DbListModel::SortMode DbListModel::getSortMode() const
{
    return sortMode;
}

void DbListModel::setSortMode(const QString& sortMode)
{
    if (sortMode == "LikeDbTree")
        this->sortMode = SortMode::LikeDbTree;
    else if (sortMode == "Alphabetical")
        this->sortMode = SortMode::Alphabetical;
    else if (sortMode == "AlphabeticalCaseInsensitive")
        this->sortMode = SortMode::AlphabeticalCaseInsensitive;
    else
        this->sortMode = SortMode::ConnectionOrder;

    sort();
}

QString DbListModel::getSortModeString() const
{
    switch (sortMode)
    {
        case DbListModel::SortMode::LikeDbTree:
            return "LikeDbTree";
        case DbListModel::SortMode::Alphabetical:
            return "Alphabetical";
        case DbListModel::SortMode::AlphabeticalCaseInsensitive:
            return "AlphabeticalCaseInsensitive";
        case DbListModel::SortMode::ConnectionOrder:
            break;
    }
    return "ConnectionOrder";
}

void DbListModel::setCombo(QComboBox* combo)
{
    comboBox = combo;
}

void DbListModel::sort()
{
    dbList = unsortedList;
    switch (sortMode)
    {
        case DbListModel::SortMode::LikeDbTree:
        {
            DbTreeComparer comparer;
            std::sort(dbList.begin(), dbList.end(), comparer);
            break;
        }
        case DbListModel::SortMode::Alphabetical:
        {
            AlphaComparer comparer;
            std::sort(dbList.begin(), dbList.end(), comparer);
            break;
        }
        case DbListModel::SortMode::AlphabeticalCaseInsensitive:
        {
            AlphaComparer comparer(Qt::CaseInsensitive);
            std::sort(dbList.begin(), dbList.end(), comparer);
            break;
        }
        case DbListModel::SortMode::ConnectionOrder:
            break;
    }
}

void DbListModel::dbConnected(Db* db)
{
    QString current;
    if (comboBox)
        current = comboBox->currentText();

    beginResetModel();
    unsortedList += db;
    sort();
    endResetModel();

    if (!current.isNull())
        comboBox->setCurrentText(current);
    else
        comboBox->setCurrentText(dbList.first()->getName());
}

void DbListModel::dbDisconnected(Db* db)
{
    QString current;
    int newIdx = -1;
    if (comboBox)
    {
        if (db->getName() == comboBox->currentText())
            newIdx = 0;
        else
            current = comboBox->currentText();
    }

    beginResetModel();
    dbList.removeAll(db);
    unsortedList.removeAll(db);
    endResetModel();

    if (!current.isNull())
        comboBox->setCurrentText(current);
    else if (newIdx > -1)
        comboBox->setCurrentIndex(newIdx);

    if (dbList.isEmpty())
        emit listCleared();
}

DbListModel::DbTreeComparer::DbTreeComparer()
{
    // TODO when sorting or D&D databases in the tree, this should be updated
    QList<DbTreeItem*> allItems = DBTREE->getModel()->getAllItemsAsFlatList();
    dbTreeOrder.clear();
    for (DbTreeItem* item : allItems)
    {
        if (item->getType() != DbTreeItem::Type::DB)
            continue;

        dbTreeOrder << item->text();
    }
}

bool DbListModel::DbTreeComparer::operator()(Db* db1, Db* db2)
{
    return dbTreeOrder.indexOf(db1->getName()) < dbTreeOrder.indexOf(db2->getName());
}

DbListModel::AlphaComparer::AlphaComparer(Qt::CaseSensitivity cs) :
    cs(cs)
{
}

bool DbListModel::AlphaComparer::operator()(Db* db1, Db* db2)
{
    return db1->getName().compare(db2->getName(), cs) < 0;
}
