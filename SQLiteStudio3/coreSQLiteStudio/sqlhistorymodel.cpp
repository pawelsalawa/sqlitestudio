#include "sqlhistorymodel.h"

SqlHistoryModel::SqlHistoryModel(QObject *parent, QSqlDatabase* db) :
    QSqlQueryModel(parent), db(db)
{
    refresh();
    setHeaderData(0, Qt::Horizontal, tr("Database", "sql history header"));
    setHeaderData(1, Qt::Horizontal, tr("Execution date", "sql history header"));
    setHeaderData(2, Qt::Horizontal, tr("Time spent", "sql history header"));
    setHeaderData(3, Qt::Horizontal, tr("Rows affected", "sql history header"));
    setHeaderData(4, Qt::Horizontal, tr("SQL", "sql history header"));
}

void SqlHistoryModel::refresh()
{
    setQuery(query, *db);
}

QVariant SqlHistoryModel::data(const QModelIndex& index, int role) const
{
    if (role == Qt::TextAlignmentRole && (index.column() == 2 || index.column() == 3))
        return (int)(Qt::AlignRight|Qt::AlignVCenter);

    return QSqlQueryModel::data(index, role);
}
