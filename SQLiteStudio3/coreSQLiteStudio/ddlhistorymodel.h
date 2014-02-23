#ifndef DDLHISTORYMODEL_H
#define DDLHISTORYMODEL_H

#include "coreSQLiteStudio_global.h"
#include <QSortFilterProxyModel>

class QSqlDatabase;

class API_EXPORT DdlHistoryModel : public QSortFilterProxyModel
{
        Q_OBJECT

    public:
        explicit DdlHistoryModel(QObject *parent, QSqlDatabase* db);

        void refresh();
        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

        QString getDbNameForFilter() const;
        void setDbNameForFilter(const QString& value);
        QStringList getDbNames() const;

    private:
        QSqlDatabase* db;
        QAbstractItemModel* internalModel = nullptr;

    signals:
        void refreshed();
};

#endif // DDLHISTORYMODEL_H
