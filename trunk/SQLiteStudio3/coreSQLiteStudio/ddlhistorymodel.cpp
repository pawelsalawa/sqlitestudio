#include "ddlhistorymodel.h"
#include <QSqlQueryModel>
#include <QSet>

DdlHistoryModel::DdlHistoryModel(QObject *parent, QSqlDatabase* db) :
    QSortFilterProxyModel(parent), db(db)
{
    internalModel = new QSqlQueryModel(this);
    setSourceModel(internalModel);

    setFilterKeyColumn(0);
    setDynamicSortFilter(true);

    refresh();
    setHeaderData(0, Qt::Horizontal, tr("Database name", "ddl history header"));
    setHeaderData(1, Qt::Horizontal, tr("Database file", "ddl history header"));
    setHeaderData(2, Qt::Horizontal, tr("Date of execution", "ddl history header"));
    setHeaderData(3, Qt::Horizontal, tr("Changes", "ddl history header"));
}

void DdlHistoryModel::refresh()
{
    static const QString query =
            "SELECT dbname,"
            "       file,"
            "       date(timestamp, 'unixepoch') AS date,"
            "       count(*)"
            "  FROM ddl_history"
            " GROUP BY dbname, file, date"
            " ORDER BY date DESC";

    dynamic_cast<QSqlQueryModel*>(internalModel)->setQuery(query, *db);

    emit refreshed();
}

QVariant DdlHistoryModel::data(const QModelIndex& index, int role) const
{
    if (role == Qt::TextAlignmentRole && (index.column() == 2 || index.column() == 3))
        return (int)(Qt::AlignRight|Qt::AlignVCenter);

    return QSortFilterProxyModel::data(index, role);
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

    QStringList nameList = dbNames.toList();
    qSort(nameList);
    return nameList;
}
