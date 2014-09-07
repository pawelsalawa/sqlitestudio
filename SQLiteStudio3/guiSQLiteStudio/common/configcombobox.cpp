#include "configcombobox.h"

ConfigComboBox::ConfigComboBox(QWidget *parent) :
    QComboBox(parent)
{
}

QVariant ConfigComboBox::getModelName() const
{
    return modelName;
}

void ConfigComboBox::setModelName(QVariant arg)
{
    modelName = arg;
}
