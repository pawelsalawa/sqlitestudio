#include "configradiobutton.h"

ConfigRadioButton::ConfigRadioButton(QWidget* parent) :
    QRadioButton(parent)
{
    connect(this, SIGNAL(toggled(bool)), this, SLOT(handleToggled(bool)));
}

QVariant ConfigRadioButton::getAssignedValue() const
{
    return assignedValue;
}

void ConfigRadioButton::setAssignedValue(const QVariant& value)
{
    assignedValue = value;
}

void ConfigRadioButton::handleToggled(bool checked)
{
    if (handlingSlot)
        return;

    if (checked)
        emit toggledOn(assignedValue);
    else
        emit toggledOff(assignedValue);
}

void ConfigRadioButton::alignToValue(const QVariant& value)
{
    handlingSlot = true;
    setChecked(value == assignedValue);
    handlingSlot = false;
}

