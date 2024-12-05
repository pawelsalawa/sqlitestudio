#include "multieditorbool.h"
#include <QVBoxLayout>
#include <QCheckBox>
#include <QVariant>

QStringList MultiEditorBool::validValues;

MultiEditorBool::MultiEditorBool(QWidget* parent)
    : MultiEditorWidget(parent)
{
    setLayout(new QVBoxLayout());
    checkBox = new QCheckBox();
    layout()->addWidget(checkBox);
#if QT_VERSION < QT_VERSION_CHECK(6, 7, 0)
    connect(checkBox, &QCheckBox::stateChanged, this, &MultiEditorBool::stateChanged);
#else
    connect(checkBox, &QCheckBox::checkStateChanged, this, &MultiEditorBool::stateChanged);
#endif
}

void MultiEditorBool::staticInit()
{
    validValues << "true" << "false"
                << "yes" << "no"
                << "on" << "off"
                << "1" << "0";
}

void MultiEditorBool::setValue(const QVariant& value)
{
    switch (value.userType())
    {
        case QMetaType::Bool:
        case QMetaType::Int:
        case QMetaType::LongLong:
        case QMetaType::UInt:
        case QMetaType::ULongLong:
            boolValue = value.toBool();
            upperCaseValue = false;
            valueFormat = BOOL;
            break;
        default:
            boolValue = valueFromString(value.toString());
            break;
    }

    updateLabel();
    checkBox->setChecked(boolValue);
}

bool MultiEditorBool::valueFromString(const QString& strValue)
{
    if (strValue.isEmpty())
    {
        upperCaseValue = false;
        valueFormat = BOOL;
        return false;
    }

    int idx = validValues.indexOf(strValue.toLower());
    if (idx < 0)
    {
        upperCaseValue = false;
        valueFormat = BOOL;
        return true;
    }

    upperCaseValue = strValue[0].isUpper();
    switch (idx)
    {
        case 0:
        case 1:
            valueFormat = TRUE_FALSE;
            break;
        case 2:
        case 3:
            valueFormat = YES_NO;
            break;
        case 4:
        case 5:
            valueFormat = ON_OFF;
            break;
        case 6:
        case 7:
            valueFormat = ONE_ZERO;
            break;
    }
    return !(bool)(idx % 2);
}

QVariant MultiEditorBool::getValue()
{
    QString value;
    switch (valueFormat)
    {
        case MultiEditorBool::TRUE_FALSE:
            value = boolValue ? "true" : "false";
            break;
        case MultiEditorBool::ON_OFF:
            value = boolValue ? "on" : "off";
            break;
        case MultiEditorBool::YES_NO:
            value = boolValue ? "yes" : "no";
            break;
        case MultiEditorBool::ONE_ZERO:
        case MultiEditorBool::BOOL:
            value = boolValue ? "1" : "0";
            break;
    }

    if (value.isNull())
        value = boolValue ? "1" : "0";

    if (upperCaseValue)
        value = value.toUpper();

    return value;
}

void MultiEditorBool::setReadOnly(bool value)
{
    readOnly = value;
}

QList<QWidget*> MultiEditorBool::getNoScrollWidgets()
{
    QList<QWidget*> list;
    list << checkBox;
    return list;
}

void MultiEditorBool::focusThisWidget()
{
    checkBox->setFocus();
}

void MultiEditorBool::updateLabel()
{
    checkBox->setText(getValue().toString());
}

#if QT_VERSION < QT_VERSION_CHECK(6, 7, 0)
void MultiEditorBool::stateChanged(int state)
{
    if (readOnly && ((bool)state) != boolValue)
#else
void MultiEditorBool::stateChanged(Qt::CheckState state)
{
    if (readOnly && (state == Qt::Checked) != boolValue)
#endif
    {
        checkBox->setChecked(boolValue);
        return;
    }

    boolValue = checkBox->isChecked();
    updateLabel();
    emit valueModified();
}

MultiEditorWidget* MultiEditorBoolPlugin::getInstance()
{
    return new MultiEditorBool();
}

bool MultiEditorBoolPlugin::validFor(const DataType& dataType)
{
    switch (dataType.getType())
    {
        case DataType::BOOLEAN:
            return true;
        case DataType::BLOB:
        case DataType::BIGINT:
        case DataType::DECIMAL:
        case DataType::DOUBLE:
        case DataType::INTEGER:
        case DataType::INT:
        case DataType::NUMERIC:
        case DataType::REAL:
        case DataType::NONE:
        case DataType::STRING:
        case DataType::TEXT:
        case DataType::CHAR:
        case DataType::VARCHAR:
        case DataType::DATE:
        case DataType::DATETIME:
        case DataType::TIME:
        case DataType::ANY:
        case DataType::unknown:
            break;
    }
    return false;
}

int MultiEditorBoolPlugin::getPriority(const DataType& dataType)
{
    switch (dataType.getType())
    {
        case DataType::BOOLEAN:
            return 1;
        case DataType::BLOB:
        case DataType::ANY:
        case DataType::BIGINT:
        case DataType::DECIMAL:
        case DataType::DOUBLE:
        case DataType::INTEGER:
        case DataType::INT:
        case DataType::NUMERIC:
        case DataType::REAL:
        case DataType::NONE:
        case DataType::STRING:
        case DataType::TEXT:
        case DataType::CHAR:
        case DataType::VARCHAR:
        case DataType::DATE:
        case DataType::DATETIME:
        case DataType::TIME:
        case DataType::unknown:
            break;
    }
    return 100;
}

QString MultiEditorBoolPlugin::getTabLabel()
{
    return tr("Boolean");
}
