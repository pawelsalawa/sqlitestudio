#include "multieditor.h"
#include "multieditortext.h"
#include "multieditornumeric.h"
#include "multieditordatetime.h"
#include "multieditordate.h"
#include "multieditortime.h"
#include "multieditorbool.h"
#include "multieditorhex.h"
#include "mainwindow.h"
#include "common/unused.h"
#include "services/notifymanager.h"
#include "services/pluginmanager.h"
#include "multieditorwidgetplugin.h"
#include "uiconfig.h"
#include "dialogs/configdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QTabBar>
#include <QLabel>
#include <QCheckBox>
#include <QVariant>
#include <QEvent>
#include <QGraphicsColorizeEffect>
#include <QToolButton>
#include <QDebug>

static QHash<QString,bool> missingEditorPluginsAlreadyWarned;

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

    configBtn = new QToolButton();
    configBtn->setToolTip(tr("Configure editors for this data type"));
    configBtn->setIcon(ICONS.CONFIGURE);
    configBtn->setFocusPolicy(Qt::NoFocus);
    configBtn->setAutoRaise(true);
    configBtn->setEnabled(false);
    connect(configBtn, SIGNAL(clicked()), this, SLOT(configClicked()));
    tabs->setCornerWidget(configBtn);

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

void MultiEditor::addEditor(MultiEditorWidget* editorWidget)
{
    editorWidget->setReadOnly(readOnly);
    connect(editorWidget, &MultiEditorWidget::valueModified, this, &MultiEditor::invalidateValue);
    editors << editorWidget;
    tabs->addTab(editorWidget, editorWidget->getTabLabel().replace("&", "&&"));
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

    stateLabel->setVisible(readOnly);
    nullCheck->setEnabled(!readOnly);
    updateVisibility();
    updateLabel();
}

void MultiEditor::setDeletedRow(bool value)
{
    deleted = value;
    updateLabel();
}

void MultiEditor::setDataType(const SqlQueryModelColumn::DataType& dataType)
{
    this->dataType = dataType;

    foreach (MultiEditorWidget* editorWidget, getEditorTypes(dataType))
        addEditor(editorWidget);

    showTab(0);
    configBtn->setEnabled(true);
}

void MultiEditor::loadBuiltInEditors()
{
    PLUGINS->loadBuiltInPlugin(new MultiEditorBoolPlugin);
    PLUGINS->loadBuiltInPlugin(new MultiEditorDateTimePlugin);
    PLUGINS->loadBuiltInPlugin(new MultiEditorDatePlugin);
    PLUGINS->loadBuiltInPlugin(new MultiEditorHexPlugin);
    PLUGINS->loadBuiltInPlugin(new MultiEditorTextPlugin);
    PLUGINS->loadBuiltInPlugin(new MultiEditorTimePlugin);
    PLUGINS->loadBuiltInPlugin(new MultiEditorNumericPlugin);
}

QList<MultiEditorWidget*> MultiEditor::getEditorTypes(const SqlQueryModelColumn::DataType& dataType)
{
    QList<MultiEditorWidget*> editors;

    QString typeStr = dataType.typeStr.trimmed().toUpper();
    QHash<QString,QVariant> editorsOrder = CFG_UI.General.DataEditorsOrder.get();
    if (editorsOrder.contains(typeStr))
    {
        MultiEditorWidgetPlugin* plugin;
        for (const QString& editorPluginName : editorsOrder[typeStr].toStringList())
        {
            plugin = dynamic_cast<MultiEditorWidgetPlugin*>(PLUGINS->getLoadedPlugin(editorPluginName));
            if (!plugin)
            {
                if (!missingEditorPluginsAlreadyWarned.contains(editorPluginName))
                {
                    notifyWarn(tr("Data editor plugin '%1' not loaded, while it is defined for editing '%1' data type."));
                    missingEditorPluginsAlreadyWarned[editorPluginName] = true;
                }
                continue;
            }

            editors << plugin->getInstance();
        }
    }

    if (editors.size() > 0)
        return editors;

    //
    // Prepare default list of editors
    //
    QList<MultiEditorWidgetPlugin*> plugins = PLUGINS->getLoadedPlugins<MultiEditorWidgetPlugin>();

    typedef QPair<int,MultiEditorWidget*> EditorWithPriority;

    QList<EditorWithPriority> sortedEditors;
    EditorWithPriority editorWithPrio;
    for (MultiEditorWidgetPlugin* plugin : plugins)
    {
        if (!plugin->validFor(dataType))
            continue;

        editorWithPrio.first = plugin->getPriority(dataType);
        editorWithPrio.second = plugin->getInstance();
        sortedEditors << editorWithPrio;
    }

    qSort(sortedEditors.begin(), sortedEditors.end(), [=](const EditorWithPriority& ed1, const EditorWithPriority& ed2) -> bool
    {
        return ed1.first < ed2.first;
    });

    for (const EditorWithPriority& e : sortedEditors)
        editors << e.second;

    return editors;
}

void MultiEditor::configClicked()
{
    ConfigDialog config(MAINWINDOW);
    config.configureDataEditors(dataType.typeStr);
    config.exec();
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

void MultiEditor::updateLabel()
{
    if (deleted)
        stateLabel->setText("<i>"+tr("Deleted", "multieditor")+"<i>");
    else if (readOnly)
        stateLabel->setText("<i>"+tr("Read only", "multieditor")+"<i>");
    else
        stateLabel->setText("");
}

QVariant MultiEditor::getValueOmmitNull() const
{
    return dynamic_cast<MultiEditorWidget*>(tabs->currentWidget())->getValue();
}
