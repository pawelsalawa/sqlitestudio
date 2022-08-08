#include "querymodel.h"
#include "db/db.h"
#include "common/unused.h"
#include "db/sqlquery.h"

QueryModel::QueryModel(Db* db, QObject *parent) :
    QAbstractTableModel(parent), db(db)
{
}

void QueryModel::refresh()
{
    if (!db || !db->isOpen())
        return;

    beginResetModel();
    loadedRows.clear();
    SqlQueryPtr results = db->exec(query);
    for (SqlResultsRowPtr& row : results->getAll())
        loadedRows += row;

    columns = results->columnCount();
    endResetModel();

    emit refreshed();
}

QVariant QueryModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    int rowIdx = index.row();
    if (rowIdx < loadedRows.size())
        return loadedRows[rowIdx]->value(index.column());

    return QVariant();
}

int QueryModel::rowCount(const QModelIndex& parent) const
{
    UNUSED(parent);
    return loadedRows.size();
}

int QueryModel::columnCount(const QModelIndex& parent) const
{
    UNUSED(parent);
    return columns;
}

QString QueryModel::getQuery() const
{
    return query;
}

void QueryModel::setQuery(const QString& value)
{
    query = value;
    refresh();
}
