#include "intvalidator.h"

IntValidator::IntValidator(QObject *parent) :
    QIntValidator(parent)
{
}

IntValidator::IntValidator(int min, int max, QObject* parent)
    : QIntValidator(min, max, parent)
{
}

void IntValidator::fixup(QString& input) const
{
    QIntValidator::fixup(input);
    if (input.trimmed().isEmpty())
        input = QString::number(defaultValue);

    bool ok;
    int val = input.toInt(&ok);
    if (!ok)
        return;

    if (val < bottom())
        input = QString::number(bottom());
    else if (val > top())
        input = QString::number(top());
}

int IntValidator::getDefaultValue() const
{
    return defaultValue;
}

void IntValidator::setDefaultValue(int value)
{
    defaultValue = value;
}
