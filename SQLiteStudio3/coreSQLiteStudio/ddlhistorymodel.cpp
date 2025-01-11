#include "ddlhistorymodel.h"
#include "querymodel.h"
#include "common/utils.h"
#include <QSet>
#include <QDebug>

DdlHistoryModel::DdlHistoryModel(Db* db, QObject *parent) :
    QSortFilterProxyModel(parent)
{
    static const QString query =
            "SELECT dbname,"
            "       file,"
            "       date(timestamp, 'unixepoch') AS date,"
            "       count(*)"
            "  FROM ddl_history"
            " GROUP BY dbname, file, date"
            " ORDER BY date DESC";

    internalModel = new QueryModel(db, this);
    setSourceModel(internalModel);
    connect(internalModel, SIGNAL(refreshed()), this, SIGNAL(refreshed()));

    setFilterKeyColumn(0);
    setDynamicSortFilter(true);

    internalModel->setQuery(query);
}

QVariant DdlHistoryModel::data(const QModelIndex& index, int role) const
{
    if (role == Qt::TextAlignmentRole && (index.column() == 2 || index.column() == 3))
        return (int)(Qt::AlignRight|Qt::AlignVCenter);

    return QSortFilterProxyModel::data(index, role);
}

void DdlHistoryModel::refresh()
{
    internalModel->refresh();

}

void DdlHistoryModel::setDbNameForFilter(const QString& value)
{
    setFilterWildcard("*"+value+"*");
}

QStringList DdlHistoryModel::getDbNames() const
{
    QSet<QString> dbNames;
    for (int row = 0; row < rowCount(); row++)
        dbNames << data(index(row, 0)).toString();

    QStringList nameList = dbNames.values();
    ::sSort(nameList);
    return nameList;
}

QVariant DdlHistoryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
    {
        switch (section)
        {
            case 0:
                return tr("Database name", "ddl history header");
            case 1:
                return tr("Database file", "ddl history header");
            case 2:
                return tr("Date of execution", "ddl history header");
            case 3:
                return tr("Changes", "ddl history header");
        }
        return QVariant();
    }
    return QSortFilterProxyModel::headerData(section, orientation, role);
}
