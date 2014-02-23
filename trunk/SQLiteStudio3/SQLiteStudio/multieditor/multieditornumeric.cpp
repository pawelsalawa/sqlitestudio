#include "multieditornumeric.h"
#include "common/numericspinbox.h"
#include <QVariant>
#include <QVBoxLayout>

MultiEditorNumeric::MultiEditorNumeric(QWidget* parent)
    : MultiEditorWidget(parent)
{
    setLayout(new QVBoxLayout());
    spinBox = new NumericSpinBox();
    layout()->addWidget(spinBox);

    connect(spinBox, &NumericSpinBox::modified, this, &MultiEditorNumeric::modified);

    setFocusProxy(spinBox);
}

void MultiEditorNumeric::setValue(const QVariant& value)
{
    spinBox->setValue(value);
}

QVariant MultiEditorNumeric::getValue()
{
    return spinBox->getValue();
}

void MultiEditorNumeric::setReadOnly(bool value)
{
    spinBox->setReadOnly(value);
}

QList<QWidget*> MultiEditorNumeric::getNoScrollWidgets()
{
    QList<QWidget*> list;
    list << spinBox;
    return list;
}

void MultiEditorNumeric::modified()
{
    emit valueModified();
}
