#ifndef SQLHISTORYMODEL_H
#define SQLHISTORYMODEL_H

#include "querymodel.h"

class Db;

class SqlHistoryModel : public QueryModel
{
        Q_OBJECT

    public:
        SqlHistoryModel(Db* db, QObject *parent = nullptr);

        QVariant data(const QModelIndex& index, int role) const;
        QVariant headerData(int section, Qt::Orientation orientation, int role) const;
};

#endif // SQLHISTORYMODEL_H
