#ifndef SQLVIEWMODEL_H
#define SQLVIEWMODEL_H

#include "sqlquerymodel.h"

class SqlViewModel : public SqlQueryModel
{
    public:
        SqlViewModel(QObject *parent = 0);

        QString generateSelectQueryForItems(const QList<SqlQueryItem*>& items);
        void setView(const QString& view);

    private:
        QString view;
};

#endif // SQLVIEWMODEL_H
