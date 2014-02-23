#include "multieditorhex.h"
#include "qhexedit2/qhexedit.h"
#include <QVBoxLayout>

MultiEditorHex::MultiEditorHex()
{
    setLayout(new QVBoxLayout());
    hexEdit = new QHexEdit();
    layout()->addWidget(hexEdit);

    //hexEdit->setTabChangesFocus(true);

    connect(hexEdit, &QHexEdit::dataChanged, this, &MultiEditorHex::modificationChanged);
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

QList<QWidget*> MultiEditorHex::getNoScrollWidgets()
{
    return QList<QWidget*>();
}

void MultiEditorHex::modificationChanged()
{
    emit valueModified();
}
