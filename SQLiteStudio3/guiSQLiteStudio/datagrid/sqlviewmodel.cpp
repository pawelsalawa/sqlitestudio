#include "sqlviewmodel.h"
#include "querygenerator.h"

SqlViewModel::SqlViewModel(QObject *parent) :
    SqlDataSourceQueryModel(parent)
{
}

QString SqlViewModel::generateSelectQueryForItems(const QList<SqlQueryItem*>& items)
{
    QHash<QString, QVariantList> values = toValuesGroupedByColumns(items);

    QueryGenerator generator;
    return generator.generateSelectFromView(db, view, values);
}

void SqlViewModel::setDatabaseAndView(const QString& database, const QString& view)
{
    this->database = database;
    this->view = view;
    //setQuery("SELECT * FROM "+getDataSource());
    updateTablesInUse(view);
}

QString SqlViewModel::getDataSource()
{
    return getDatabasePrefix() + wrapObjIfNeeded(view);
}
