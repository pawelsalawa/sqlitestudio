#ifndef SQLVIEWMODEL_H
#define SQLVIEWMODEL_H

#include "sqldatasourcequerymodel.h"

class SqlViewModel : public SqlDataSourceQueryModel
{
    public:
        explicit SqlViewModel(QObject *parent = 0);

        QString getView() const;

        QString generateSelectQueryForItems(const QList<SqlQueryItem*>& items);
        void setDatabaseAndView(const QString& database, const QString& view);

    protected:
        QString getDataSource();

    private:
        QString view;
};

#endif // SQLVIEWMODEL_H
