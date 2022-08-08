#ifndef SQLQUERYMODEL_H
#define SQLQUERYMODEL_H

#include "db/sqlresultsrow.h"
#include <QList>
#include <QAbstractTableModel>

class Db;

class API_EXPORT QueryModel : public QAbstractTableModel
{
        Q_OBJECT

    public:
        using QAbstractItemModel::fetchMore;
        using QAbstractItemModel::canFetchMore;

        QueryModel(Db* db, QObject *parent = nullptr);

        virtual void refresh();
        QVariant data(const QModelIndex& index, int role) const;
        int rowCount(const QModelIndex& parent) const;
        int columnCount(const QModelIndex& parent) const;

        QString getQuery() const;
        void setQuery(const QString& value);

    private:
        void fetchMore();
        bool canFetchMore() const;

        QString query;
        Db* db = nullptr;
        QList<SqlResultsRowPtr> loadedRows;
        int columns = 0;

    signals:
        void refreshed();
};

#endif // SQLQUERYMODEL_H
