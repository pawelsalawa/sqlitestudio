#include "multieditornumeric.h"
#include "common/numericspinbox.h"
#include <QVariant>
#include <QVBoxLayout>
#include <QDebug>

MultiEditorNumeric::MultiEditorNumeric(QWidget* parent)
    : MultiEditorWidget(parent)
{
    setLayout(new QVBoxLayout());
    spinBox = new NumericSpinBox();
    layout()->addWidget(spinBox);

    connect(spinBox, SIGNAL(modified()), this, SIGNAL(valueModified()));

    setFocusProxy(spinBox);
}

void MultiEditorNumeric::setValue(const QVariant& value)
{
    spinBox->setValue(value, false);
}

QVariant MultiEditorNumeric::getValue()
{
    return spinBox->getValue();
}

void MultiEditorNumeric::setReadOnly(bool value)
{
    spinBox->setReadOnly(value);
}

void MultiEditorNumeric::focusThisWidget()
{
    spinBox->setFocus();
}

QList<QWidget*> MultiEditorNumeric::getNoScrollWidgets()
{
    QList<QWidget*> list;
    list << spinBox;
    return list;
}

MultiEditorWidget*MultiEditorNumericPlugin::getInstance()
{
    return new MultiEditorNumeric();
}

bool MultiEditorNumericPlugin::validFor(const DataType& dataType)
{
    switch (dataType.getType())
    {
        case DataType::BIGINT:
        case DataType::DECIMAL:
        case DataType::DOUBLE:
        case DataType::INTEGER:
        case DataType::INT:
        case DataType::NUMERIC:
        case DataType::REAL:
            return true;
        case DataType::BOOLEAN:
        case DataType::BLOB:
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

int MultiEditorNumericPlugin::getPriority(const DataType& dataType)
{
    switch (dataType.getType())
    {
        case DataType::BIGINT:
        case DataType::DECIMAL:
        case DataType::DOUBLE:
        case DataType::INTEGER:
        case DataType::INT:
        case DataType::NUMERIC:
        case DataType::REAL:
            return 1;
        case DataType::BOOLEAN:
        case DataType::BLOB:
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
    return 10;
}

QString MultiEditorNumericPlugin::getTabLabel()
{
    return tr("Number", "numeric multi editor tab name");
}
