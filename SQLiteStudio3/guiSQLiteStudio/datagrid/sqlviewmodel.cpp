#include "sqlviewmodel.h"
#include "querygenerator.h"

SqlViewModel::SqlViewModel(QObject *parent) :
    SqlQueryModel(parent)
{
}

QString SqlViewModel::generateSelectQueryForItems(const QList<SqlQueryItem*>& items)
{
    QHash<QString, QVariantList> values = toValuesGroupedByColumns(items);

    QueryGenerator generator;
    return generator.generateSelectFromView(db, view, values);
}

void SqlViewModel::setView(const QString& view)
{
    this->view = view;
}
