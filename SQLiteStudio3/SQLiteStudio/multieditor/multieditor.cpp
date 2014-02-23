#include "multieditor.h"
#include "multieditortext.h"
#include "multieditornumeric.h"
#include "multieditordatetime.h"
#include "multieditordate.h"
#include "multieditortime.h"
#include "multieditorbool.h"
#include "multieditorhex.h"
#include "unused.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QTabBar>
#include <QLabel>
#include <QCheckBox>
#include <QVariant>
#include <QEvent>
#include <QGraphicsColorizeEffect>
#include <QDebug>

MultiEditor::MultiEditor(QWidget *parent) :
    QWidget(parent)
{
    init();
}

void MultiEditor::init()
{
    QVBoxLayout* vbox = new QVBoxLayout();
    vbox->setMargin(margins);
    vbox->setSpacing(spacing);
    setLayout(vbox);

    QWidget* top = new QWidget();
    layout()->addWidget(top);

    QHBoxLayout* hbox = new QHBoxLayout();
    hbox->setMargin(0);
    hbox->setSpacing(0);
    top->setLayout(hbox);

    nullCheck = new QCheckBox(tr("Null value", "multieditor"));
    hbox->addWidget(nullCheck);

    hbox->addStretch();

    stateLabel = new QLabel();
    hbox->addWidget(stateLabel);

    hbox->addSpacing(50);

    tabs = new QTabWidget();
    layout()->addWidget(tabs);
    tabs->tabBar()->installEventFilter(this);

    QGraphicsColorizeEffect* effect = new QGraphicsColorizeEffect();
    effect->setColor(Qt::black);
    effect->setStrength(0.5);
    nullEffect = effect;
    tabs->setGraphicsEffect(effect);

    connect(tabs, &QTabWidget::currentChanged, this, &MultiEditor::tabChanged);
    connect(nullCheck, &QCheckBox::stateChanged, this, &MultiEditor::nullStateChanged);
    connect(this, SIGNAL(modified()), this, SLOT(setModified()));
}

void MultiEditor::tabChanged(int idx)
{
    int prevTab = currentTab;
    currentTab = idx;

    MultiEditorWidget* newEditor = editors[idx];
    newEditor->setFocus();

    if (prevTab < 0)
        return;

    if (newEditor->isUpToDate())
        return;

    MultiEditorWidget* prevEditor = editors[prevTab];
    newEditor->setValue(prevEditor->getValue());
    newEditor->setUpToDate(true);
}

void MultiEditor::nullStateChanged(int state)
{
    bool checked = (state == Qt::Checked);

    if (checked)
        valueBeforeNull = getValueOmmitNull();

    updateNullEffect();
    updateValue(checked ? QVariant() : valueBeforeNull);

    if (!checked)
        valueBeforeNull.clear();

    tabs->setEnabled(!checked);
    emit modified();
}

void MultiEditor::invalidateValue()
{
    if (invalidatingDisabled)
        return;

    QObject* obj = sender();
    if (!obj)
    {
        qWarning() << "No sender object while invalidating MultiEditor value.";
        return;
    }

    QWidget* editorWidget;
    for (int i = 0; i < tabs->count(); i++)
    {
        editorWidget = tabs->widget(i);
        if (editorWidget == obj)
            continue; // skip sender

        dynamic_cast<MultiEditorWidget*>(editorWidget)->setUpToDate(false);
    }

    emit modified();
}

void MultiEditor::setModified()
{
    valueModified = true;
}

void MultiEditor::addEditor(MultiEditor::BuiltInEditor editor)
{
    QString label;
    MultiEditorWidget* editorWidget = nullptr;
    switch (editor)
    {
        case MultiEditor::TEXT:
            editorWidget = new MultiEditorText();
            label = tr("Text", "multieditor label");
            break;
        case MultiEditor::BLOB:
            editorWidget = new MultiEditorHex();
            label = tr("Hex", "multieditor label");
            break;
        case MultiEditor::NUMERIC:
            editorWidget = new MultiEditorNumeric();
            label = tr("Number", "multieditor label");
            break;
        case MultiEditor::BOOLEAN:
            editorWidget = new MultiEditorBool();
            label = tr("Boolean", "multieditor label");
            break;
        case MultiEditor::DATE:
            editorWidget = new MultiEditorDate();
            label = tr("Date", "multieditor label");
            break;
        case MultiEditor::TIME:
            editorWidget = new MultiEditorTime();
            label = tr("Time", "multieditor label");
            break;
        case MultiEditor::DATETIME:
            editorWidget = new MultiEditorDateTime();
            label = tr("Date and time", "multieditor label");
            break;
    }
    editorWidget->setReadOnly(readOnly);
    connect(editorWidget, &MultiEditorWidget::valueModified, this, &MultiEditor::invalidateValue);
    editors << editorWidget;
    tabs->addTab(editorWidget, label);
    editorWidget->installEventFilter(this);
}

void MultiEditor::showTab(int idx)
{
    tabs->setCurrentIndex(idx);
}

void MultiEditor::setValue(const QVariant& value)
{
    nullCheck->setChecked(!value.isValid() || value.isNull());
    updateVisibility();
    updateValue(value);
    valueModified = false;
}

QVariant MultiEditor::getValue() const
{
    if (nullCheck->isChecked())
        return QVariant();

    return getValueOmmitNull();
}

bool MultiEditor::isModified() const
{
    return valueModified;
}

bool MultiEditor::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::Wheel)
    {
        QWidget::event(event);
        return true;
    }

    return QWidget::eventFilter(obj, event);
}

bool MultiEditor::getReadOnly() const
{
    return readOnly;
}

void MultiEditor::setReadOnly(bool value)
{
    readOnly = value;

    for (int i = 0; i < tabs->count(); i++)
        dynamic_cast<MultiEditorWidget*>(tabs->widget(i))->setReadOnly(value);

    stateLabel->setText("<i>"+tr("Read only", "multieditor")+"<i>");
    stateLabel->setVisible(readOnly);
    nullCheck->setEnabled(!readOnly);
    updateVisibility();
}

void MultiEditor::setDeletedRow(bool value)
{
    setReadOnly(value);
    stateLabel->setText("<i>"+tr("Deleted", "multieditor")+"<i>");
}

void MultiEditor::setDataType(const SqlQueryModelColumn::DataType& dataType)
{
    foreach (BuiltInEditor editorType, getEditorTypes(dataType))
        addEditor(editorType);

    showTab(0);
}

QList<MultiEditor::BuiltInEditor> MultiEditor::getEditorTypes(const SqlQueryModelColumn::DataType& dataType)
{
    QList<MultiEditor::BuiltInEditor> alternativeEditors;
    MultiEditor::BuiltInEditor editor = MultiEditor::TEXT;
    if (dataType.type)
    {
        switch (dataType.type)
        {
            case DataType::INT:
            case DataType::BIGINT:
            case DataType::INTEGER:
            case DataType::DOUBLE:
            case DataType::REAL:
            case DataType::DECIMAL:
            case DataType::NUMERIC:
                editor = MultiEditor::NUMERIC;
                alternativeEditors << MultiEditor::TEXT;
                break;
            case DataType::BOOLEAN:
                editor = MultiEditor::BOOLEAN;
                alternativeEditors << MultiEditor::TEXT;
                break;
            case DataType::TEXT:
            case DataType::VARCHAR:
            case DataType::CHAR:
            case DataType::STRING:
            case DataType::NONE:
            case DataType::_NULL:
                editor = MultiEditor::TEXT;
                break;
            case DataType::BLOB:
                editor = MultiEditor::TEXT;
                alternativeEditors << MultiEditor::BLOB;
                break;
            case DataType::DATE:
                editor = MultiEditor::DATE;
                alternativeEditors << MultiEditor::DATETIME;
                alternativeEditors << MultiEditor::TEXT;
                break;
            case DataType::TIME:
                editor = MultiEditor::TIME;
                alternativeEditors << MultiEditor::TEXT;
                break;
            case DataType::DATETIME:
                editor = MultiEditor::DATETIME;
                alternativeEditors << MultiEditor::TEXT;
                break;
        }
    }

    QList<MultiEditor::BuiltInEditor> editors;
    editors << editor;
    editors += alternativeEditors;
    return editors;
}

void MultiEditor::updateVisibility()
{
    tabs->setVisible(!readOnly || !nullCheck->isChecked());
    nullCheck->setVisible(!readOnly || nullCheck->isChecked());
    updateNullEffect();
}

void MultiEditor::updateNullEffect()
{
    nullEffect->setEnabled(tabs->isVisible() && nullCheck->isChecked());
    if (tabs->isVisible())
    {
        for (int i = 0; i < tabs->count(); i++)
            dynamic_cast<MultiEditorWidget*>(tabs->widget(i))->update();

        nullEffect->update();
    }
}

void MultiEditor::updateValue(const QVariant& newValue)
{
    invalidatingDisabled = true;
    MultiEditorWidget* editorWidget;
    for (int i = 0; i < tabs->count(); i++)
    {
        editorWidget = dynamic_cast<MultiEditorWidget*>(tabs->widget(i));
        editorWidget->setValue(newValue);
        editorWidget->setUpToDate(true);
    }
    invalidatingDisabled = false;
}

QVariant MultiEditor::getValueOmmitNull() const
{
    return dynamic_cast<MultiEditorWidget*>(tabs->currentWidget())->getValue();
}
