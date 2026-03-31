#include "datawidgetmapper.h"
#include <QAbstractItemModel>
#include <QWidget>
#include <QDebug>

DataWidgetMapper::DataWidgetMapper(QObject *parent) :
    QObject(parent)
{
}
QAbstractItemModel* DataWidgetMapper::getModel() const
{
    return model;
}

void DataWidgetMapper::setModel(QAbstractItemModel* value)
{
    model = value;
}

void DataWidgetMapper::addMapping(QWidget* widget, int modelColumn, const QString& propertyName, const QList<PropertyAndRole>& extraProperties)
{
    MappingEntry* entry = new MappingEntry;
    entry->columnIndex = modelColumn;
    entry->widget = widget;
    entry->properties = {PropertyAndRole{propertyName, Qt::EditRole}};
    entry->properties += extraProperties;
    mappings[widget] = entry;
}

void DataWidgetMapper::addMapping(QWidget* widget, const QString& propertyName, int role)
{
    if (!mappings.contains(widget))
    {
        qCritical() << "Widget" << widget << "is not mapped to any column. Cannot add extra property mapping.";
        return;
    }
    mappings[widget]->properties << PropertyAndRole{propertyName, role};
}

void DataWidgetMapper::clearMapping()
{
    for (MappingEntry* entry : mappings.values())
        delete entry;

    mappings.clear();
}

int DataWidgetMapper::getCurrentIndex() const
{
    return currentIndex;
}

int DataWidgetMapper::mappedSection(QWidget* widget) const
{
    if (mappings.contains(widget))
        return mappings[widget]->columnIndex;

    return -1;
}

void DataWidgetMapper::loadFromModel()
{
    for (MappingEntry* entry : mappings.values())
    {
        QModelIndex idx = model->index(currentIndex, entry->columnIndex);
        for (const PropertyAndRole& prop : entry->properties)
        {
            QVariant data = model->data(idx, prop.second);
            entry->widget->setProperty(prop.first.toLatin1().constData(), data);
        }
    }
}

DataWidgetMapper::SubmitFilter DataWidgetMapper::getSubmitFilter() const
{
    return submitFilter;
}

void DataWidgetMapper::setSubmitFilter(const SubmitFilter& value)
{
    submitFilter = value;
}

void DataWidgetMapper::setCurrentIndex(int rowIndex)
{
    if (!model)
        return;

    if (rowIndex < 0)
        return;

    if (rowIndex >= model->rowCount())
        return;

    if (model->rowCount() == 0)
        return;

    currentIndex = rowIndex;
    loadFromModel();
    emit currentIndexChanged(rowIndex);
}

void DataWidgetMapper::toFirst()
{
    setCurrentIndex(0);
}

void DataWidgetMapper::toLast()
{
    if (!model)
        return;

    setCurrentIndex(model->rowCount() - 1);
}

void DataWidgetMapper::toNext()
{
    setCurrentIndex(currentIndex + 1);
}

void DataWidgetMapper::toPrevious()
{
    setCurrentIndex(currentIndex - 1);
}

void DataWidgetMapper::submit()
{
    for (MappingEntry* entry : mappings.values())
    {
        if (submitFilter && !submitFilter(entry->widget))
            continue;

        for (const PropertyAndRole& prop : entry->properties)
        {
            QModelIndex idx = model->index(currentIndex, entry->columnIndex);
            QVariant value = entry->widget->property(prop.first.toLatin1().constData());
            model->setData(idx, value, prop.second);
        }
    }
}

void DataWidgetMapper::revert()
{
    if (!model)
        return;

    if (currentIndex < 0)
        return;

    loadFromModel();
}

