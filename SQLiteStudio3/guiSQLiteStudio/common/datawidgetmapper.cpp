#include "datawidgetmapper.h"
#include <QAbstractItemModel>
#include <QWidget>

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

void DataWidgetMapper::addMapping(QWidget* widget, int modelColumn, const QString& propertyName)
{
    MappingEntry* entry = new MappingEntry;
    entry->columnIndex = modelColumn;
    entry->widget = widget;
    entry->propertyName = propertyName;
    mappings[widget] = entry;
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
    QModelIndex idx;
    QVariant data;
    for (MappingEntry* entry : mappings.values())
    {
        idx = model->index(currentIndex, entry->columnIndex);
        data = model->data(idx, Qt::EditRole);
        entry->widget->setProperty(entry->propertyName.toLatin1().constData(), data);
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
    QModelIndex idx;
    QVariant value;
    for (MappingEntry* entry : mappings.values())
    {
        if (submitFilter && !submitFilter(entry->widget))
            continue;

        idx = model->index(currentIndex, entry->columnIndex);
        value = entry->widget->property(entry->propertyName.toLatin1().constData());
        model->setData(idx, value, Qt::EditRole);
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

