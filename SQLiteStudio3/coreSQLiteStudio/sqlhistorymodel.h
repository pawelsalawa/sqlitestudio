#ifndef SQLHISTORYMODEL_H
#define SQLHISTORYMODEL_H

#include <QSqlQueryModel>

class QSqlDatabase;

class SqlHistoryModel : public QSqlQueryModel
{
    public:
        explicit SqlHistoryModel(QObject *parent, QSqlDatabase* db);

        void refresh();
        QVariant data(const QModelIndex& index, int role) const;

    private:
        QSqlDatabase* db;

        static constexpr const char* query =
                "SELECT dbname, datetime(date, 'unixepoch'), (time_spent / 1000.0)||'s', rows, sql FROM sqleditor_history ORDER BY date DESC";
};

#endif // SQLHISTORYMODEL_H
