#include "multieditorfk.h"
#include "datagrid/fkcombobox.h"
#include <QVBoxLayout>
#include <QLineEdit>

MultiEditorFk::MultiEditorFk(QWidget* parent)
    : MultiEditorWidget(parent)
{
    setLayout(new QVBoxLayout());
    comboBox = new FkComboBox(this);
    comboBox->setEditable(false);
    layout()->addWidget(comboBox);

    connect(comboBox, SIGNAL(valueModified()), this, SIGNAL(valueModified()));

    setFocusProxy(comboBox);
}

void MultiEditorFk::initFkCombo(Db* db, SqlQueryModelColumn* columnModel)
{
    comboBox->init(db, columnModel);
}

void MultiEditorFk::setValue(const QVariant& value)
{
    comboBox->setValue(value);
}

QVariant MultiEditorFk::getValue()
{
    return comboBox->getValue();
}

void MultiEditorFk::setReadOnly(bool value)
{
    comboBox->setDisabled(value);
}

void MultiEditorFk::focusThisWidget()
{
    comboBox->setFocus();
}

QList<QWidget*> MultiEditorFk::getNoScrollWidgets()
{
    QList<QWidget*> list;
    list << comboBox;
    return list;
}
