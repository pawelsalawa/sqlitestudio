#ifndef SQLQUERYROWNUMMODEL_H
#define SQLQUERYROWNUMMODEL_H

#include "guiSQLiteStudio_global.h"
#include <QAbstractItemModel>

class GUI_API_EXPORT SqlQueryRowNumModel : public QAbstractItemModel
{
        Q_OBJECT
    public:
        SqlQueryRowNumModel(QAbstractItemModel *value, QObject *parent = 0);

        QModelIndex index(int row, int column, const QModelIndex &parent) const;
        QModelIndex parent(const QModelIndex &child) const;
        int rowCount(const QModelIndex &parent) const;
        int columnCount(const QModelIndex &parent) const;
        QVariant data(const QModelIndex &index, int role) const;
        QVariant headerData(int section, Qt::Orientation orientation, int role) const;

        void setRowNumBase(int value);

    private:
        int rowNumBase = 1;
        QAbstractItemModel* mainModel;
};

#endif // SQLQUERYROWNUMMODEL_H
