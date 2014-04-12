#include "sqlhistorymodel.h"
#include "common/global.h"
#include "db/db.h"

SqlHistoryModel::SqlHistoryModel(Db* db, QObject *parent) :
    QueryModel(db, parent)
{
    static_char* query = "SELECT dbname, datetime(date, 'unixepoch'), (time_spent / 1000.0)||'s', rows, sql "
                         "FROM sqleditor_history ORDER BY date DESC";

    setQuery(query);

    setHeaderData(0, Qt::Horizontal, tr("Database", "sql history header"));
    setHeaderData(1, Qt::Horizontal, tr("Execution date", "sql history header"));
    setHeaderData(2, Qt::Horizontal, tr("Time spent", "sql history header"));
    setHeaderData(3, Qt::Horizontal, tr("Rows affected", "sql history header"));
    setHeaderData(4, Qt::Horizontal, tr("SQL", "sql history header"));
}

QVariant SqlHistoryModel::data(const QModelIndex& index, int role) const
{
    if (role == Qt::TextAlignmentRole && (index.column() == 2 || index.column() == 3))
        return (int)(Qt::AlignRight|Qt::AlignVCenter);

    return QueryModel::data(index, role);
}
