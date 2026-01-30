#include "multieditorhex.h"
#include "qhexedit2/qhexedit.h"
#include "common/unused.h"
#include <QVBoxLayout>

MultiEditorHex::MultiEditorHex()
{
    setLayout(new QVBoxLayout());
    hexEdit = new QHexEdit();
    layout()->addWidget(hexEdit);

    connect(hexEdit, SIGNAL(dataChanged()), this, SLOT(modificationChanged()));
    setFocusProxy(hexEdit);
}

MultiEditorHex::~MultiEditorHex()
{
}

void MultiEditorHex::setValue(const QVariant& value)
{
    hexEdit->setData(value.toByteArray());
}

QVariant MultiEditorHex::getValue()
{
    return hexEdit->data();
}

void MultiEditorHex::setReadOnly(bool value)
{
    hexEdit->setReadOnly(value);
}

void MultiEditorHex::focusThisWidget()
{
    hexEdit->setFocus();
}

QList<QWidget*> MultiEditorHex::getNoScrollWidgets()
{
    return QList<QWidget*>();
}

void MultiEditorHex::modificationChanged()
{
    emit valueModified();
}

MultiEditorWidget*MultiEditorHexPlugin::getInstance()
{
    return new MultiEditorHex();
}

bool MultiEditorHexPlugin::validFor(const DataType& dataType)
{
    UNUSED(dataType);
    return true;
}

int MultiEditorHexPlugin::getPriority(const QVariant& value, const DataType& dataType)
{
    // Returning 3 to force opening hex editor on double-click for binary types or QByteArray values,
    // but leaving 1 & 2 for specialized editors (e.g. image editor, etc.)
    if (dataType.isBinary() && value.userType() == QMetaType::QByteArray)
        return 3;

    switch (dataType.getType())
    {
        case DataType::BLOB:
            return 3;
        case DataType::ANY:
        case DataType::NONE:
        case DataType::TEXT:
        case DataType::CHAR:
        case DataType::STRING:
        case DataType::VARCHAR:
        case DataType::unknown:
        {
            if (value.userType() == QMetaType::QByteArray)
                return 3;

            break;
        }
        case DataType::BIGINT:
        case DataType::DECIMAL:
        case DataType::DOUBLE:
        case DataType::INTEGER:
        case DataType::INT:
        case DataType::NUMERIC:
        case DataType::REAL:
        case DataType::BOOLEAN:
        case DataType::DATE:
        case DataType::DATETIME:
        case DataType::TIME:
            break;
    }
    return 100;
}

QString MultiEditorHexPlugin::getTabLabel()
{
    return tr("Hex");
}
