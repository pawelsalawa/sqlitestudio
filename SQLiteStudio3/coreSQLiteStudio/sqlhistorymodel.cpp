#include "sqlhistorymodel.h"
#include "common/global.h"
#include "db/db.h"

SqlHistoryModel::SqlHistoryModel(Db* db, QObject *parent) :
    QueryModel(db, parent)
{
    static_char* query = "SELECT id, dbname, datetime(date, 'unixepoch', 'localtime'), (time_spent / 1000.0)||'s', rows, sql "
                         "FROM sqleditor_history ORDER BY date DESC";

    setQuery(query);
}

QVariant SqlHistoryModel::data(const QModelIndex& index, int role) const
{
    if (role == Qt::TextAlignmentRole && (index.column() == 2 || index.column() == 3))
        return (int)(Qt::AlignRight|Qt::AlignVCenter);

    return QueryModel::data(index, role);
}

QVariant SqlHistoryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return QueryModel::headerData(section, orientation, role);

    switch (section)
    {
        case 0:
            return "";
        case 1:
            return tr("Database", "sql history header");
        case 2:
            return tr("Execution date", "sql history header");
        case 3:
            return tr("Time spent", "sql history header");
        case 4:
            return tr("Rows affected", "sql history header");
        case 5:
            return tr("SQL", "sql history header");
    }

    return QueryModel::headerData(section, orientation, role);
}
