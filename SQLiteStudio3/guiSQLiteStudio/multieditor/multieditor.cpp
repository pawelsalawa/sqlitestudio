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
#include <QFileDialog>

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
    font.setPointSizeF(font.pointSizeF() * 1.5);
    cornerLabel->setFont(font);
    cornerLabel->setFrameStyle(QFrame::NoFrame);
    hbox->addWidget(cornerLabel);
    cornerLabel->setVisible(false);

    hbox->addSpacing(10);

    nullCheck = new QCheckBox(tr("Null value", "multieditor"));
    hbox->addWidget(nullCheck);

    stateLabel = new QLabel();
    hbox->addWidget(stateLabel);

    int minWidth = qMax(nullCheck->sizeHint().width(), stateLabel->sizeHint().width());
    nullCheck->setMinimumWidth(minWidth);
    stateLabel->setMinimumWidth(minWidth);

    saveToFileButton = createButton(ICONS.SAVE_FILE, tr("Save this value to a file"), SLOT(saveFile()));
    hbox->addWidget(saveToFileButton);

    loadFromFileButton = createButton(ICONS.OPEN_FILE, tr("Load this value from a file"), SLOT(openFile()));
    hbox->addWidget(loadFromFileButton);

    tabs = new QTabWidget();
    layout()->addWidget(tabs);
    tabs->tabBar()->installEventFilter(this);

    switch (tabsMode)
    {
        case CONFIGURABLE:
        {
            configBtn = createButton(ICONS.CONFIGURE, tr("Configure editors for this data type"), SLOT(configClicked()));
            hbox->addWidget(configBtn);
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

    hbox->addStretch(); // squeeze to the left

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
    if (idx < 0)
        return;

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

    for (int i = 0; i < tabs->count(); i++)
    {
        QWidget* editorWidget = tabs->widget(i);
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
        QAction* addTabAction = addTabMenu->actions() | FIND_FIRST(a,
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
    nullCheck->setChecked(isNull(value));
    valueBeforeNull = value;
    updateVisibility();
    updateValue(value);
    valueModified = false;
}

void MultiEditor::setValueAndType(const QVariant& value, const DataType& dataType, bool selectTabByValuePriority)
{
    if (selectTabByValuePriority)
        setDataType(dataType, value);
    else
        setDataType(dataType);

    setValue(value);
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
    loadFromFileButton->setEnabled(!readOnly);
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
    setDataType(dataType, QVariant());
}

void MultiEditor::setDataType(const DataType& dataType, const QVariant& forValue)
{
    this->dataType = dataType;

    int bestEditorIdx = 0;
    for (MultiEditorWidget*& editorWidget : getEditorTypes(dataType, forValue, &bestEditorIdx))
        addEditor(editorWidget);

    showTab(bestEditorIdx);
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

QString MultiEditor::getEditorFileFilter()
{
    if (!tabs->currentWidget())
        return QString();

    return dynamic_cast<MultiEditorWidget*>(tabs->currentWidget())->getPreferredFileFilter();
}

int MultiEditor::getCornerLabelWidth() const
{
    return cornerLabel->sizeHint().width();
}

void MultiEditor::adjustCornerLabelMinWidth(int value)
{
    cornerLabel->setMinimumWidth(value);
}

void MultiEditor::setSaveButtonVisible(bool visible)
{
    saveToFileButton->setVisible(visible);
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

QString MultiEditor::getFileDialogFilter(FileDialogFilters filter)
{
    switch (filter)
    {
        case MultiEditor::ALL_FILES:
            return tr("All files (*)");
        case MultiEditor::TEXT_FILES:
            return tr("Text files (*.txt *.log *.csv *.tsv *.md *.json *.xml *.yaml *.yml)");
        case MultiEditor::SQL_FILES:
            return tr("SQL files (*.sql)");
        case MultiEditor::BINARY_DATA:
            return tr("Binary data (*.bin *.dat *.raw)");
        case MultiEditor::IMAGES:
            return tr("Images (*.jpeg *.jpg *.png *.bmp *.gif *.tiff *.jp2 *.svg *.tga *.icns *.webp *.wbmp *.mng)");
        case MultiEditor::ARCHIVES:
            return tr("Archives (*.zip *.7z *.rar *.tar *.gz *.bz2 *.xz)");
        case MultiEditor::DOCUMENTS:
            return tr("Documents (*.pdf *.rtf *.doc *.docx *.odt *.xls *.xlsx *.ods)");
        case MultiEditor::EXEUTABLES:
            return tr("Executables (*.exe *.dll *.so *.dylib)");
    }
    return QString();
}

QList<MultiEditorWidget*> MultiEditor::getEditorTypes(const DataType& dataType, const QVariant& value, int* priorityEditorIndex)
{
    QList<MultiEditorWidget*> editors;
    MultiEditorWidget* editor = nullptr;

    int editorIdx = 0;
    int currentBestPrio = 999999;
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
            if (priorityEditorIndex && currentBestPrio > 1)
            {
                int prio = plugin->getPriority(value, dataType);
                if (prio < currentBestPrio)
                {
                    currentBestPrio = prio;
                    *priorityEditorIndex = editorIdx;
                }
            }
            editor->setTabLabel(plugin->getTabLabel());
            editors << editor;
            editorIdx++;
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

        editorWithPrio.first = plugin->getPriority(value, dataType);
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

    if (priorityEditorIndex)
        *priorityEditorIndex = 0;

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
    for (int i = 0; i < tabs->count(); i++)
    {
        MultiEditorWidget* editorWidget = dynamic_cast<MultiEditorWidget*>(tabs->widget(i));
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
    if (!tabs->currentWidget())
        return QVariant(QString());

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

QString MultiEditor::getFileFilter(const QString& customFilter) const
{
    QStringList list;
    list << getFileDialogFilter(ALL_FILES);
    list << getFileDialogFilter(TEXT_FILES);
    list << getFileDialogFilter(SQL_FILES);
    list << getFileDialogFilter(BINARY_DATA);
    list << getFileDialogFilter(IMAGES);
    list << getFileDialogFilter(ARCHIVES);
    list << getFileDialogFilter(DOCUMENTS);
    list << getFileDialogFilter(EXEUTABLES);
    if (!customFilter.isEmpty())
        list << customFilter;

    return list.join(";;");
}

QToolButton* MultiEditor::createButton(const QIcon& icon, const QString& tooltip, const char* slot)
{
    int height = nullCheck->sizeHint().height();

    QToolButton* btn = new QToolButton();
    btn->setAutoRaise(true);
    btn->setIcon(icon);
    btn->setIconSize(QSize(height, height));
    btn->setFocusPolicy(Qt::NoFocus);
    btn->setToolTip(tooltip);
    connect(btn, SIGNAL(clicked()), this, slot);
    return btn;
}

void MultiEditor::removeTab(int idx)
{
    MultiEditorWidget* editor = dynamic_cast<MultiEditorWidget*>(tabs->widget(idx));
    QString label = editor->getTabLabel();
    tabs->removeTab(idx);

    // Re-add it to menu
    MultiEditorWidgetPlugin* plugin = PLUGINS->getLoadedPlugins<MultiEditorWidgetPlugin>() |
                                      FIND_FIRST(p, {return p->getTabLabel() == label;});

    if (!plugin)
    {
        qWarning() << "Missing MultiEditorWidgetPlugin after removing its tab for label:" << label;
        return;
    }

    addPluginToMenu(plugin);
    sortAddTabMenu();
}

void MultiEditor::openFile()
{
    QString customFilter = getEditorFileFilter();
    QString dir = getFileDialogInitPath();
    QString filter = getFileFilter(customFilter);
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open"), dir, filter, customFilter.isEmpty() ? nullptr : &customFilter);
    if (fileName.isNull())
        return;

    setFileDialogInitPathByFile(fileName);

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
    {
        notifyError(tr("Could not open file %1 for reading.").arg(fileName));
        return;
    }

    QByteArray newData = file.readAll();
    file.close();

    updateValue(newData);
    emit modified();
}

void MultiEditor::saveFile()
{
    QString customFilter = getEditorFileFilter();
    QString dir = getFileDialogInitPath();
    QString filter = getFileFilter(customFilter);
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save"), dir, filter, customFilter.isEmpty() ? nullptr : &customFilter);
    if (fileName.isNull())
        return;

    setFileDialogInitPathByFile(fileName);

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly))
    {
        notifyError(tr("Could not open file %1 for writting.").arg(fileName));
        return;
    }

    bool ok = false;
    QVariant value = getValue();
    switch (value.userType())
    {
        case QMetaType::QByteArray:
        {
            QDataStream out(&file);
            QByteArray ba = value.toByteArray();
            out.writeRawData(ba.constData(), ba.size());
            ok = (out.status() == QDataStream::Ok);
            break;
        }
        case QMetaType::Int:
        case QMetaType::Bool:
        {
            QTextStream out(&file);
            out << value.toInt();
            ok = (out.status() == QTextStream::Ok);
            break;
        }
        case QMetaType::Double:
        {
            QTextStream out(&file);
            out << value.toDouble();
            ok = (out.status() == QTextStream::Ok);
            break;
        }
        case QMetaType::UInt:
        case QMetaType::LongLong:
        {
            QTextStream out(&file);
            out << value.toLongLong();
            ok = (out.status() == QTextStream::Ok);
            break;
        }
        default:
        {
            QTextStream out(&file);
            out.setEncoding(QStringConverter::Utf8);
            out << value.toString();
            ok = (out.status() == QTextStream::Ok);
            break;
        }
    }

    if (!ok)
        notifyError(tr("Could not write data into the file %1").arg(fileName));

    file.close();
}
