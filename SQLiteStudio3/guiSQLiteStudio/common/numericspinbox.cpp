#include "numericspinbox.h"
#include "common/unused.h"
#include <QLineEdit>
#include <QVariant>
#include <QDebug>

NumericSpinBox::NumericSpinBox(QWidget *parent) :
    QAbstractSpinBox(parent)
{
    connect(lineEdit(), &QLineEdit::textChanged, this, &NumericSpinBox::valueEdited);
}

void NumericSpinBox::stepBy(int steps)
{
    if (isReadOnly())
        return;

    switch (value.userType())
    {
        case QMetaType::Double:
            stepDoubleBy(steps);
            break;
        case QMetaType::Int:
        case QMetaType::LongLong:
            stepIntBy(steps);
            break;
        default:
            break;
    }
    updateText();
}

QValidator::State NumericSpinBox::validate(QString& input, int& pos) const
{
    UNUSED(input);
    UNUSED(pos);

    if (strict)
        return validateStrict(input, pos);

    return QValidator::Acceptable;
}

void NumericSpinBox::stepIntBy(int steps)
{
    qint64 intVal = value.toLongLong();
    intVal += steps;
    value = intVal;
    emit modified();
}

void NumericSpinBox::stepDoubleBy(int steps)
{
    double doubleVal = value.toDouble();
    doubleVal += steps;
    value = doubleVal;
    emit modified();
}

void NumericSpinBox::updateText()
{
    lineEdit()->setText(value.toString());
}

QValidator::State NumericSpinBox::validateStrict(QString& input, int& pos) const
{
    if (input.trimmed().isEmpty())
        return allowEmpty ? QValidator::Acceptable : QValidator::Invalid;

    QIntValidator vint;
    if (vint.validate(input, pos) != QValidator::Invalid)
        return QValidator::Acceptable;

    QDoubleValidator dint;
    if (dint.validate(input, pos) != QValidator::Invalid)
        return QValidator::Acceptable;

    return QValidator::Invalid;
}
bool NumericSpinBox::getAllowEmpty() const
{
    return allowEmpty;
}

void NumericSpinBox::setAllowEmpty(bool value)
{
    allowEmpty = value;
}


bool NumericSpinBox::isStrict() const
{
    return strict;
}

void NumericSpinBox::setStrict(bool value, bool allowEmpty)
{
    strict = value;
    this->allowEmpty = allowEmpty;
}

void NumericSpinBox::valueEdited(const QString& value)
{
    setValueInternal(value);
    emit modified();
}

QAbstractSpinBox::StepEnabled NumericSpinBox::stepEnabled() const
{
    return StepDownEnabled|StepUpEnabled;
}

QVariant NumericSpinBox::getFixedVariant(const QVariant& value)
{
    if (allowEmpty)
    {
        if (value.userType() == QMetaType::QString && value.toString().isEmpty() && !value.isNull())
            return "";

        if (value.isNull())
            return QString();
    }

    bool ok;
    qint64 longVal = value.toLongLong(&ok);
    if (ok)
        return longVal;

    return value.toDouble();
}

void NumericSpinBox::setValueInternal(const QVariant& newValue)
{
    switch (newValue.userType())
    {
        case QMetaType::QString:
            value = getFixedVariant(newValue);
            break;
        case QMetaType::Double:
        case QMetaType::Int:
        case QMetaType::LongLong:
            value = newValue;
            break;
        default:
            value = 0;
    }
}

QVariant NumericSpinBox::getValue() const
{
    return value;
}

void NumericSpinBox::setValue(const QVariant& newValue, bool nullAsZero)
{
    setValueInternal(newValue);
    if (!nullAsZero && newValue.isNull())
        value = newValue;

    updateText();
}
