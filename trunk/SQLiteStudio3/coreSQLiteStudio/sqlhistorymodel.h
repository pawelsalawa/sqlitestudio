#ifndef SQLHISTORYMODEL_H
#define SQLHISTORYMODEL_H

#include "querymodel.h"

class Db;

class SqlHistoryModel : public QueryModel
{
    public:
        SqlHistoryModel(Db* db, QObject *parent = nullptr);

        QVariant data(const QModelIndex& index, int role) const;
};

#endif // SQLHISTORYMODEL_H
