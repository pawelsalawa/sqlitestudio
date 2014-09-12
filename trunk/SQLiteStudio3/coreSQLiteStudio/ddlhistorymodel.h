#ifndef DDLHISTORYMODEL_H
#define DDLHISTORYMODEL_H

#include "coreSQLiteStudio_global.h"
#include <QSortFilterProxyModel>

class QueryModel;
class Db;

class API_EXPORT DdlHistoryModel : public QSortFilterProxyModel
{
        Q_OBJECT

    public:
        DdlHistoryModel(Db* db, QObject *parent = nullptr);

        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
        void refresh();
        QString getDbNameForFilter() const;
        void setDbNameForFilter(const QString& value);
        QStringList getDbNames() const;
        QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    private:
        QueryModel* internalModel = nullptr;

    signals:
        void refreshed();
};

#endif // DDLHISTORYMODEL_H
