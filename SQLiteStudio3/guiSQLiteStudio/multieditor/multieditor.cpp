#include "multieditor.h"
#include "multieditor/multieditorfk.h"
#include "multieditortext.h"
#include "multieditornumeric.h"
#include "multieditordatetime.h"
#include "multieditordate.h"
#include "multieditortime.h"
#include "multieditorbool.h"
#include "multieditorhex.h"
#include "mainwindow.h"
#include "iconmanager.h"
#include "services/notifymanager.h"
#include "services/pluginmanager.h"
#include "multieditorwidgetplugin.h"
#include "uiconfig.h"
#include "dialogs/configdialog.h"
#include "themetuner.h"
#include "datagrid/sqlquerymodelcolumn.h"
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
#include <QKeyEvent>

static QHash<QString,bool> missingEditorPluginsAlreadyWarned;

MultiEditor::MultiEditor(QWidget *parent, TabsMode tabsMode) :
    QWidget(parent)
{
    init(tabsMode);
}

void MultiEditor::init(TabsMode tabsMode)
{
    QVBoxLayout* vbox = new QVBoxLayout();
    vbox->setContentsMargins(margins, margins, margins, margins);
    vbox->setSpacing(spacing);
    setLayout(vbox);

    QWidget* top = new QWidget();
    layout()->addWidget(top);

    QHBoxLayout* hbox = new QHBoxLayout();
    hbox->setContentsMargins(0, 0, 0, 0);
    hbox->setSpacing(10);
    top->setLayout(hbox);

    cornerLabel = new QLabel();
    QFont font = cornerLabel->font();
    font.setBold(true);
    cornerLabel->setFont(font);
    cornerLabel->setFrameStyle(QFrame::NoFrame);
    hbox->addWidget(cornerLabel);
    cornerLabel->setVisible(false);

    nullCheck = new QCheckBox(tr("Null value", "multieditor"));
    hbox->addWidget(nullCheck);

    hbox->addStretch();

    stateLabel = new QLabel();
    hbox->addWidget(stateLabel);

    hbox->addSpacing(50);

    tabs = new QTabWidget();
    layout()->addWidget(tabs);
    tabs->tabBar()->installEventFilter(this);

    switch (tabsMode)
    {
        case CONFIGURABLE:
        {
            configBtn = new QToolButton();
            configBtn->setToolTip(tr("Configure editors for this data type"));
            configBtn->setIcon(ICONS.CONFIGURE);
            configBtn->setFocusPolicy(Qt::NoFocus);
            configBtn->setAutoRaise(true);
            configBtn->setEnabled(false);
            connect(configBtn, SIGNAL(clicked()), this, SLOT(configClicked()));
            tabs->setCornerWidget(configBtn);
            break;
        }
        case DYNAMIC:
        {
            initAddTabMenu();
            addTabBtn = new QToolButton();
            addTabBtn->setToolTip(tr("Open another tab"));
            addTabBtn->setIcon(ICONS.PLUS);
            addTabBtn->setFocusPolicy(Qt::NoFocus);
            addTabBtn->setAutoRaise(true);
            addTabBtn->setEnabled(true);
            addTabBtn->setPopupMode(QToolButton::InstantPopup);
            addTabBtn->setMenu(addTabMenu);
            tabs->setCornerWidget(addTabBtn);
            tabs->setTabsClosable(true);
            connect(tabs, &QTabWidget::tabCloseRequested, this, &MultiEditor::removeTab);
            break;
        }
        case PRECONFIGURED:
            break;
    }

    QGraphicsColorizeEffect* effect = new QGraphicsColorizeEffect();
    effect->setColor(Qt::black);
    effect->setStrength(0.5);
    nullEffect = effect;
    tabs->setGraphicsEffect(effect);

    connect(tabs, &QTabWidget::currentChanged, this, &MultiEditor::tabChanged);
#if QT_VERSION < QT_VERSION_CHECK(6, 7, 0)
    connect(nullCheck, &QCheckBox::stateChanged, this, &MultiEditor::nullStateChanged);
#else
    connect(nullCheck, &QCheckBox::checkStateChanged, this, &MultiEditor::nullStateChanged);
#endif
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

#if QT_VERSION < QT_VERSION_CHECK(6, 7, 0)
void MultiEditor::nullStateChanged(int state)
#else
void MultiEditor::nullStateChanged(Qt::CheckState state)
#endif
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

    QWidget* editorWidget = nullptr;
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
    connect(editorWidget, SIGNAL(valueModified()), this, SLOT(invalidateValue()));
    editors << editorWidget;
    tabs->addTab(editorWidget, editorWidget->getTabLabel().replace("&", "&&"));
    editorWidget->installEventFilter(this);

    connect(editorWidget, &MultiEditorWidget::aboutToBeDeleted, [this, editorWidget]()
    {
        int idx = tabs->indexOf(editorWidget);
        tabs->removeTab(idx);
    });

    if (addTabMenu)
    {
        QAction* addTabAction = findFirst<QAction>(addTabMenu->actions(), [editorWidget](QAction* a)
        {
            return a->data().toString() == editorWidget->getTabLabel();
        });

        if (addTabAction)
            addTabMenu->removeAction(addTabAction);
        else
            qWarning() << "Could not find action associated with added MultiEditorWidget:" << editorWidget->getTabLabel();
    }
}

void MultiEditor::showTab(int idx)
{
    tabs->setCurrentIndex(idx);
}

void MultiEditor::setValue(const QVariant& value)
{
    nullCheck->setChecked(!value.isValid() || value.isNull());
    valueBeforeNull = value;
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

void MultiEditor::setDataType(const DataType& dataType)
{
    this->dataType = dataType;

    for (MultiEditorWidget*& editorWidget : getEditorTypes(dataType))
        addEditor(editorWidget);

    showTab(0);
    if (configBtn)
        configBtn->setEnabled(true);
}

void MultiEditor::enableFk(Db* db, SqlQueryModelColumn* column)
{
    MultiEditorFk* fkEditor = new MultiEditorFk();
    fkEditor->initFkCombo(db, column);
    fkEditor->setTabLabel(tr("Foreign Key"));
    addEditor(fkEditor);
}

void MultiEditor::focusThisEditor()
{
    MultiEditorWidget* w = dynamic_cast<MultiEditorWidget*>(tabs->currentWidget());
    if (!w)
        return;

    w->focusThisWidget();
}

void MultiEditor::setCornerLabel(const QString &label)
{
    cornerLabel->setText(label);
    cornerLabel->setVisible(!label.isNull());
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

QList<MultiEditorWidget*> MultiEditor::getEditorTypes(const DataType& dataType)
{
    QList<MultiEditorWidget*> editors;
    MultiEditorWidget* editor = nullptr;

    QString typeStr = dataType.toString().trimmed().toUpper();
    QHash<QString,QVariant> editorsOrder = CFG_UI.General.DataEditorsOrder.get();
    if (editorsOrder.contains(typeStr))
    {
        MultiEditorWidgetPlugin* plugin = nullptr;
        for (QString& editorPluginName : editorsOrder[typeStr].toStringList())
        {
            plugin = dynamic_cast<MultiEditorWidgetPlugin*>(PLUGINS->getLoadedPlugin(editorPluginName));
            if (!plugin)
            {
                if (!missingEditorPluginsAlreadyWarned.contains(editorPluginName))
                {
                    notifyWarn(tr("Data editor plugin '%1' not loaded, while it is defined for editing '%2' data type.")
                                   .arg(editorPluginName, typeStr));
                    missingEditorPluginsAlreadyWarned[editorPluginName] = true;
                }
                continue;
            }

            editor = plugin->getInstance();
            editor->setTabLabel(plugin->getTabLabel());
            editors << editor;
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
        editorWithPrio.second->setTabLabel(plugin->getTabLabel());
        sortedEditors << editorWithPrio;
    }

    sSort(sortedEditors, [=](const EditorWithPriority& ed1, const EditorWithPriority& ed2) -> bool
    {
        return ed1.first < ed2.first;
    });

    for (EditorWithPriority& e : sortedEditors)
        editors << e.second;

    return editors;
}

void MultiEditor::configClicked()
{
    ConfigDialog config(MAINWINDOW);
    config.configureDataEditors(dataType.toString());
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
    MultiEditorWidget* editorWidget = nullptr;
    for (int i = 0; i < tabs->count(); i++)
    {
        editorWidget = dynamic_cast<MultiEditorWidget*>(tabs->widget(i));
        setValueToWidget(editorWidget, newValue);
    }
    invalidatingDisabled = false;
}

void MultiEditor::setValueToWidget(MultiEditorWidget* editorWidget, const QVariant& newValue)
{
    editorWidget->setValue(newValue);
    editorWidget->setUpToDate(true);
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

void MultiEditor::initAddTabMenu()
{
    addTabMenu = new QMenu(addTabBtn);
    for (MultiEditorWidgetPlugin*& plugin : PLUGINS->getLoadedPlugins<MultiEditorWidgetPlugin>())
        addPluginToMenu(plugin);

    sortAddTabMenu();
}

void MultiEditor::addPluginToMenu(MultiEditorWidgetPlugin* plugin)
{
    QAction* addTabAction = addTabMenu->addAction(plugin->getTabLabel());
    addTabAction->setData(plugin->getTabLabel()); // for display-independent identification of action to avoid ampersand issue
    connect(addTabAction, &QAction::triggered, [plugin, this]()
    {
        MultiEditorWidget* editor = plugin->getInstance();
        editor->setTabLabel(plugin->getTabLabel());
        addEditor(editor);
        setValueToWidget(editor, valueBeforeNull);
        showTab(tabs->count() - 1);
    });
}

void MultiEditor::sortAddTabMenu()
{
    QList<QAction*> editorActions = addTabMenu->actions();
    std::sort(editorActions.begin(), editorActions.end(), [](QAction* a1, QAction* a2)
    {
        return a1->data().toString().compare(a2->data().toString(), Qt::CaseInsensitive) < 0;
    });

    for (QAction* action : editorActions)
        addTabMenu->removeAction(action);

    addTabMenu->insertActions(nullptr, editorActions);
}

void MultiEditor::removeTab(int idx)
{
    MultiEditorWidget* editor = dynamic_cast<MultiEditorWidget*>(tabs->widget(idx));
    QString label = editor->getTabLabel();
    tabs->removeTab(idx);

    // Re-add it to menu
    MultiEditorWidgetPlugin* plugin = findFirst<MultiEditorWidgetPlugin>(
                PLUGINS->getLoadedPlugins<MultiEditorWidgetPlugin>(),
                [label](MultiEditorWidgetPlugin* p) {return p->getTabLabel() == label;}
            );

    if (!plugin)
    {
        qWarning() << "Missing MultiEditorWidgetPlugin after removing its tab for label:" << label;
        return;
    }

    addPluginToMenu(plugin);
    sortAddTabMenu();
}
