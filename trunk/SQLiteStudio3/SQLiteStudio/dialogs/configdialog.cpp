#include "configdialog.h"
#include "ui_configdialog.h"
#include "services/config.h"
#include "uiconfig.h"
#include "customconfigwidgetplugin.h"
#include "services/pluginmanager.h"
#include "formmanager.h"
#include "services/codeformatter.h"
#include "plugins/codeformatterplugin.h"
#include "configwidgets/styleconfigwidget.h"
#include "configwidgets/combodatawidget.h"
#include "configwidgets/listtostringlisthash.h"
#include "iconmanager.h"
#include "common/userinputfilter.h"
#include "multieditor/multieditorwidget.h"
#include "multieditor/multieditorwidgetplugin.h"
#include "mainwindow.h"
#include "common/unused.h"
#include "sqlitestudio.h"
#include "configmapper.h"
#include "datatype.h"
#include <QSignalMapper>
#include <QLineEdit>
#include <QSpinBox>
#include <QDebug>
#include <QComboBox>
#include <QStyleFactory>
#include <QFile>
#include <QDir>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QListWidget>
#include <QTableWidget>
#include <QDesktopServices>
#include <QtUiTools/QUiLoader>

#define GET_FILTER_STRING(Widget, WidgetType, Method) \
    if (qobject_cast<WidgetType*>(Widget))\
        return qobject_cast<WidgetType*>(Widget)->Method() + " " + Widget->toolTip();\

#define GET_FILTER_STRING2(Widget, WidgetType) \
    WidgetType* w##WidgetType = qobject_cast<WidgetType*>(widget);\
    if (w##WidgetType)\
        return getFilterString(w##WidgetType) + " " + Widget->toolTip();

ConfigDialog::ConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigDialog)
{
    init();
}

ConfigDialog::~ConfigDialog()
{
    delete ui;
    safe_delete(configMapper);
}

void ConfigDialog::configureDataEditors(const QString& dataTypeString)
{
    ui->categoriesWidget->setVisible(false);
    ui->stackedWidget->setCurrentWidget(ui->dataEditorsPage);

    for (int i = 0; i < ui->dataEditorsTypesList->count(); i++)
    {
        if (ui->dataEditorsTypesList->item(i)->text() == dataTypeString.toUpper())
        {
            ui->dataEditorsTypesList->setCurrentRow(i);
            return;
        }
    }

    addDataType(dataTypeString.toUpper());
}

QString ConfigDialog::getFilterString(QWidget *widget)
{
    // Common code for widgets with single method call
    GET_FILTER_STRING(widget, QLabel, text);
    GET_FILTER_STRING(widget, QAbstractButton, text);
    GET_FILTER_STRING(widget, QLineEdit, text);
    GET_FILTER_STRING(widget, QTextEdit, toPlainText);
    GET_FILTER_STRING(widget, QPlainTextEdit, toPlainText);
    GET_FILTER_STRING(widget, QGroupBox, title);

    // Widgets needs a little more than single method call
    GET_FILTER_STRING2(widget, QComboBox);
    GET_FILTER_STRING2(widget, QTreeWidget);
    GET_FILTER_STRING2(widget, QListWidget);
    GET_FILTER_STRING2(widget, QTableWidget);

    return QString::null;
}

QString ConfigDialog::getFilterString(QComboBox *widget)
{
    QStringList items;
    for (int i = 0; i < widget->count(); i++)
        items << widget->itemText(i);

    return items.join(" ");
}

QString ConfigDialog::getFilterString(QTreeWidget *widget)
{
    QList<QTreeWidgetItem*> items = widget->findItems("*", Qt::MatchWildcard|Qt::MatchRecursive);
    QStringList strList;
    foreach (QTreeWidgetItem* item, items)
        for (int i = 0; i < widget->columnCount(); i++)
            strList << item->text(i) + " " + item->toolTip(0);

    return strList.join(" ");
}

QString ConfigDialog::getFilterString(QListWidget *widget)
{
    QList<QListWidgetItem*> items = widget->findItems("*", Qt::MatchWildcard|Qt::MatchRecursive);
    QStringList strList;
    foreach (QListWidgetItem* item, items)
        strList << item->text() + " " + item->toolTip();

    return strList.join(" ");
}

QString ConfigDialog::getFilterString(QTableWidget *widget)
{
    QList<QTableWidgetItem*> items = widget->findItems("*", Qt::MatchWildcard|Qt::MatchRecursive);
    QStringList strList;
    foreach (QTableWidgetItem* item, items)
         strList << item->text() + " " + item->toolTip();

    return strList.join(" ");
}

void ConfigDialog::init()
{
    ui->setupUi(this);
    setWindowIcon(ICONS.CONFIGURE);

    configMapper = new ConfigMapper(CfgMain::getPersistableInstances());
    connect(configMapper, SIGNAL(modified()), this, SLOT(markModified()));

    ui->categoriesFilter->setClearButtonEnabled(true);
    UserInputFilter* filter = new UserInputFilter(ui->categoriesFilter, this, SLOT(applyFilter(QString)));
    filter->setDelay(500);

    ui->stackedWidget->setCurrentWidget(ui->generalPage);
    initPageMap();
    initInternalCustomConfigWidgets();
    initPlugins();
    initPluginsPage();
    initFormatterPlugins();
    initDataEditors();

    connect(ui->categoriesTree, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), this, SLOT(switchPage(QTreeWidgetItem*)));
    connect(ui->previewTabs, SIGNAL(currentChanged(int)), this, SLOT(updateStylePreview()));
    connect(ui->activeStyleCombo, SIGNAL(currentTextChanged(QString)), this, SLOT(updateStylePreview()));
    connect(ui->buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()), this, SLOT(apply()));
    connect(ui->hideBuiltInPluginsCheck, SIGNAL(toggled(bool)), this, SLOT(updateBuiltInPluginsVisibility()));

    ui->activeStyleCombo->addItems(QStyleFactory::keys());

    connect(ui->stackedWidget, SIGNAL(currentChanged(int)), this, SLOT(pageSwitched()));

    ui->hideBuiltInPluginsCheck->setChecked(true);

    load();
    updateStylePreview();
}

void ConfigDialog::load()
{
    updatingDataEditorItem = true;
    configMapper->loadToWidget(ui->stackedWidget);
    updatingDataEditorItem = false;
    setModified(false);
}

void ConfigDialog::save()
{
    MainWindow::getInstance()->setStyle(ui->activeStyleCombo->currentText());

    QString loadedPlugins = collectLoadedPlugins();
    CFG->beginMassSave();
    CFG_CORE.General.LoadedPlugins.set(loadedPlugins);
    configMapper->saveFromWidget(ui->stackedWidget, true);
    CFG->commitMassSave();
}

void ConfigDialog::markModified()
{
    setModified(true);
}

void ConfigDialog::setModified(bool modified)
{
    modifiedFlag = modified;
    updateModified();
}

void ConfigDialog::updateModified()
{
    ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(modifiedFlag);
}

void ConfigDialog::applyFilter(const QString &filter)
{
    QColor normalColor = ui->categoriesTree->palette().color(QPalette::Active, QPalette::WindowText);
    QColor disabledColor = ui->categoriesTree->palette().color(QPalette::Disabled, QPalette::WindowText);
    if (filter.isEmpty())
    {
        foreach (QTreeWidgetItem* item, getAllCategoryItems())
            item->setForeground(0, normalColor);

        return;
    }

    QList<QWidget*> widgets = ui->stackedWidget->findChildren<QWidget*>();
    QList<QWidget*> matchedWidgets;
    foreach (QWidget* widget, widgets)
    {
        if (getFilterString(widget).contains(filter, Qt::CaseInsensitive))
            matchedWidgets << widget;
    }

    QHash<QWidget*, QTreeWidgetItem*> pageToCategoryItem = buildPageToCategoryItemMap();
    QSet<QTreeWidgetItem*> matchedCategories;
    foreach (QWidget* page, pageToCategoryItem.keys())
    {
        foreach (QWidget* matched, matchedWidgets)
        {
            if (page->isAncestorOf(matched))
            {
                if (!pageToCategoryItem.contains(page))
                {
                    qCritical() << "Page" << page << "not on page-to-category-item mapping.";
                    continue;
                }

                matchedCategories << pageToCategoryItem[page];
                break;
            }
        }
    }

    foreach (QTreeWidgetItem* item, getAllCategoryItems())
        item->setForeground(0, disabledColor);

    foreach (QTreeWidgetItem* item, matchedCategories)
    {
        item->setForeground(0, normalColor);
        while ((item = item->parent()) != nullptr)
            item->setForeground(0, normalColor);
    }
}

QHash<QWidget*, QTreeWidgetItem*> ConfigDialog::buildPageToCategoryItemMap() const
{
    QHash<QString,QTreeWidgetItem*> pageNameToCategoryItem;
    foreach (QTreeWidgetItem* item, getAllCategoryItems())
        pageNameToCategoryItem[item->statusTip(0)] = item;

    QWidget* page;
    QHash<QWidget*,QTreeWidgetItem*> pageToCategoryItem;
    for (int i = 0; i < ui->stackedWidget->count(); i++)
    {
        page = ui->stackedWidget->widget(i);
        pageToCategoryItem[page] = pageNameToCategoryItem[page->objectName()];
    }
    return pageToCategoryItem;
}

QList<QTreeWidgetItem *> ConfigDialog::getAllCategoryItems() const
{
    return ui->categoriesTree->findItems("*", Qt::MatchWildcard|Qt::MatchRecursive);
}

QList<MultiEditorWidgetPlugin*> ConfigDialog::getDefaultEditorsForType(DataType::Enum dataType)
{
    QList<MultiEditorWidgetPlugin*> plugins = PLUGINS->getLoadedPlugins<MultiEditorWidgetPlugin>();
    DataType modelDataType;
    modelDataType.setType(dataType);

    typedef QPair<int,MultiEditorWidgetPlugin*> PluginWithPriority;
    QList<PluginWithPriority> sortedPlugins;
    PluginWithPriority editorWithPrio;
    for (MultiEditorWidgetPlugin* plugin : plugins)
    {
        if (!plugin->validFor(modelDataType))
            continue;

        editorWithPrio.first = plugin->getPriority(modelDataType);
        editorWithPrio.second = plugin;
        sortedPlugins << editorWithPrio;
    }

    qSort(sortedPlugins.begin(), sortedPlugins.end(), [=](const PluginWithPriority& p1, const PluginWithPriority& p2) -> bool
    {
       return p1.first < p2.first;
    });

    QList<MultiEditorWidgetPlugin*> results;
    for (const PluginWithPriority& p: sortedPlugins)
        results << p.second;

    return results;
}

void ConfigDialog::pageSwitched()
{
    if (ui->stackedWidget->currentWidget() == ui->dataEditorsPage)
    {
        updateDataTypeEditors();
        return;
    }
}

void ConfigDialog::updateDataTypeEditors()
{
    QString typeName = ui->dataEditorsTypesList->currentItem()->text();
    DataType::Enum typeEnum = DataType::fromString(typeName);
    QStringList editorsOrder = getPluginNamesFromDataTypeItem(ui->dataEditorsTypesList->currentItem());
    bool usingCustomOrder = (editorsOrder.size() > 0);
    QList<MultiEditorWidgetPlugin*> sortedPlugins;

    while (ui->dataEditorsSelectedTabs->count() > 0)
        delete ui->dataEditorsSelectedTabs->widget(0);

    ui->dataEditorsAvailableList->clear();
    if (usingCustomOrder)
        sortedPlugins = updateCustomDataTypeEditors(editorsOrder);
    else
        sortedPlugins = updateDefaultDataTypeEditors(typeEnum);

    ui->dataEditorsAvailableList->sortItems();

    for (MultiEditorWidgetPlugin* plugin : sortedPlugins)
        addDataTypeEditor(plugin);
}

QList<MultiEditorWidgetPlugin*> ConfigDialog::updateCustomDataTypeEditors(const QStringList& editorsOrder)
{
    // Building plugins list
    QList<MultiEditorWidgetPlugin*> plugins = PLUGINS->getLoadedPlugins<MultiEditorWidgetPlugin>();
    QList<MultiEditorWidgetPlugin*> enabledPlugins;
    QListWidgetItem* item;
    for (MultiEditorWidgetPlugin* plugin : plugins)
    {
        item = new QListWidgetItem(plugin->getTitle());
        item->setFlags(item->flags()|Qt::ItemIsUserCheckable);
        item->setCheckState(editorsOrder.contains(plugin->getName()) ? Qt::Checked : Qt::Unchecked);
        item->setData(QListWidgetItem::UserType, plugin->getName());
        if (item->checkState() == Qt::Checked)
            enabledPlugins << plugin;

        ui->dataEditorsAvailableList->addItem(item);
    }

    qSort(enabledPlugins.begin(), enabledPlugins.end(), [=](MultiEditorWidgetPlugin* p1, MultiEditorWidgetPlugin* p2) -> bool
    {
        return editorsOrder.indexOf(p1->getName()) < editorsOrder.indexOf(p2->getName());
    });

    return enabledPlugins;
}

QList<MultiEditorWidgetPlugin*> ConfigDialog::updateDefaultDataTypeEditors(DataType::Enum typeEnum)
{
    // Building plugins list
    QList<MultiEditorWidgetPlugin*> plugins = PLUGINS->getLoadedPlugins<MultiEditorWidgetPlugin>();
    QList<MultiEditorWidgetPlugin*> enabledPlugins = getDefaultEditorsForType(typeEnum);
    QListWidgetItem* item;
    for (MultiEditorWidgetPlugin* plugin : plugins)
    {
        item = new QListWidgetItem(plugin->getTitle());
        item->setFlags(item->flags()|Qt::ItemIsUserCheckable);
        item->setCheckState(enabledPlugins.contains(plugin) ? Qt::Checked : Qt::Unchecked);
        item->setData(QListWidgetItem::UserType, plugin->getName());
        ui->dataEditorsAvailableList->addItem(item);
    }
    return enabledPlugins;
}

void ConfigDialog::addDataTypeEditor(const QString& pluginName)
{
    MultiEditorWidgetPlugin* plugin = dynamic_cast<MultiEditorWidgetPlugin*>(PLUGINS->getLoadedPlugin(pluginName));
    if (!plugin)
    {
        qCritical() << "Could not find plugin" << pluginName << " in ConfigDialog::addDataTypeEditor()";
        return;
    }

    addDataTypeEditor(plugin);
}

void ConfigDialog::addDataTypeEditor(MultiEditorWidgetPlugin* plugin)
{
    MultiEditorWidget* editor = plugin->getInstance();
    ui->dataEditorsSelectedTabs->addTab(editor, editor->getTabLabel().replace("&", "&&"));
}

void ConfigDialog::removeDataTypeEditor(QListWidgetItem* item, const QString& pluginName)
{
    QStringList orderedPlugins = getPluginNamesFromDataTypeItem(item);
    int idx = orderedPlugins.indexOf(pluginName);
    removeDataTypeEditor(idx);
}

void ConfigDialog::removeDataTypeEditor(int idx)
{
    if (idx < 0 || idx > (ui->dataEditorsSelectedTabs->count() - 1))
    {
        qCritical() << "Index out of range in ConfigDialog::removeDataTypeEditor():" << idx << "(tabs:" << ui->dataEditorsSelectedTabs->count() << ")";
        return;
    }

    delete ui->dataEditorsSelectedTabs->widget(idx);
}

void ConfigDialog::transformDataTypeEditorsToCustomList(QListWidgetItem* typeItem)
{
    DataType::Enum dataType = DataType::fromString(typeItem->text());
    QList<MultiEditorWidgetPlugin*> plugins = getDefaultEditorsForType(dataType);

    QStringList pluginNames;
    for (MultiEditorWidgetPlugin* plugin : plugins)
        pluginNames << plugin->getName();

    setPluginNamesForDataTypeItem(typeItem, pluginNames);
}

QStringList ConfigDialog::getPluginNamesFromDataTypeItem(QListWidgetItem* typeItem)
{
    return typeItem->data(QListWidgetItem::UserType).toStringList();
}

void ConfigDialog::setPluginNamesForDataTypeItem(QListWidgetItem* typeItem, const QStringList& pluginNames)
{
    updatingDataEditorItem = true;
    typeItem->setData(QListWidgetItem::UserType, pluginNames);
    updatingDataEditorItem = false;
}

void ConfigDialog::addDataType(const QString& typeStr)
{
    QListWidgetItem* item = new QListWidgetItem(typeStr);
    item->setFlags(item->flags()|Qt::ItemIsEditable);
    ui->dataEditorsTypesList->addItem(item);
    ui->dataEditorsTypesList->setCurrentRow(ui->dataEditorsTypesList->count() - 1, QItemSelectionModel::Clear|QItemSelectionModel::SelectCurrent);
    markModified();
}

void ConfigDialog::updateDataTypeListState()
{
    bool listEditingEnabled = ui->dataEditorsTypesList->selectedItems().size() > 0 && ui->dataEditorsTypesList->currentItem()->flags().testFlag(Qt::ItemIsEditable);
    dataEditRenameAction->setEnabled(listEditingEnabled);
    dataEditDeleteAction->setEnabled(listEditingEnabled);

    bool orderEditingEnabled = ui->dataEditorsTypesList->selectedItems().size() > 0;
    ui->dataEditorsAvailableList->setEnabled(orderEditingEnabled);
    ui->dataEditorsSelectedTabs->setEnabled(orderEditingEnabled);
}

void ConfigDialog::dataEditorItemEdited(QListWidgetItem* item)
{
    if (updatingDataEditorItem)
        return;

    updatingDataEditorItem = true;
    QString txt = item->text().toUpper();
    if (DataType::getAllNames().contains(txt))
        txt += "_";

    while (ui->dataEditorsTypesList->findItems(txt, Qt::MatchExactly).size() > 1)
        txt += "_";

    item->setText(txt);
    updatingDataEditorItem = false;
}

void ConfigDialog::dataEditorAvailableChanged(QListWidgetItem* item)
{
    QListWidgetItem* typeItem = ui->dataEditorsTypesList->currentItem();
    if (!typeItem)
        return;

    QStringList pluginNames = getPluginNamesFromDataTypeItem(typeItem);
    if (pluginNames.size() == 0)
    {
        transformDataTypeEditorsToCustomList(typeItem);
        pluginNames = getPluginNamesFromDataTypeItem(typeItem);
    }

    QString pluginName = item->data(QListWidgetItem::UserType).toString();
    Qt::CheckState state = item->checkState();
    if (pluginNames.contains(pluginName) && state == Qt::Unchecked)
    {
        removeDataTypeEditor(typeItem, pluginName);
        pluginNames.removeOne(pluginName);

    }
    else if (!pluginNames.contains(pluginName) && state == Qt::Checked)
    {
        addDataTypeEditor(pluginName);
        pluginNames << pluginName;
    }

    setPluginNamesForDataTypeItem(typeItem, pluginNames);
}

void ConfigDialog::dataEditorTabsOrderChanged(int from, int to)
{
    QListWidgetItem* typeItem = ui->dataEditorsTypesList->currentItem();
    if (!typeItem)
        return;

    QStringList pluginNames = getPluginNamesFromDataTypeItem(typeItem);
    if (pluginNames.size() == 0)
    {
        transformDataTypeEditorsToCustomList(typeItem);
        pluginNames = getPluginNamesFromDataTypeItem(typeItem);
    }

    int pluginSize = pluginNames.size();
    if (from >= pluginSize || to >= pluginSize)
    {
        qCritical() << "Tabse moved out of range. in ConfigDialog::dataEditorTabsOrderChanged(). Range was: " << pluginSize << "and indexes were:" << from << to;
        return;
    }

    QString pluginName = pluginNames[from];
    pluginNames.removeAt(from);
    pluginNames.insert(to, pluginName);

    setPluginNamesForDataTypeItem(typeItem, pluginNames);
}

void ConfigDialog::addDataType()
{
    addDataType("");
    renameDataType();
}

void ConfigDialog::renameDataType()
{
    QListWidgetItem* item = ui->dataEditorsTypesList->currentItem();
    if (!item)
        return;

    ui->dataEditorsTypesList->editItem(item);
}

void ConfigDialog::delDataType()
{
    QListWidgetItem* item = ui->dataEditorsTypesList->currentItem();
    if (!item)
        return;

    int row = ui->dataEditorsTypesList->currentRow();
    delete ui->dataEditorsTypesList->takeItem(row);

    if (ui->dataEditorsTypesList->count() > 0)
    {
        if (ui->dataEditorsTypesList->count() <= row)
        {
            row--;
            if (row < 0)
                row = 0;
        }

        ui->dataEditorsTypesList->setCurrentRow(row, QItemSelectionModel::Clear|QItemSelectionModel::SelectCurrent);
    }

    updateDataTypeListState();
    markModified();
}

void ConfigDialog::dataTypesHelp()
{
    static const QString url = QStringLiteral("http://sqlitestudio.pl/wiki/index.php/User_Manual#Customizing_data_type_editors");
    QDesktopServices::openUrl(QUrl(url, QUrl::StrictMode));
}

void ConfigDialog::updateActiveFormatterState()
{
    ui->activeFormatterConfigButton->setEnabled(ui->activeFormatterCombo->currentIndex() > -1);
}

void ConfigDialog::activeFormatterChanged(const QString& title)
{
    UNUSED(title);
    updateActiveFormatterState();
}

void ConfigDialog::activeFormatterConfigurePressed()
{
    QString title = ui->activeFormatterCombo->currentText();
    QTreeWidgetItem* item = getItemByTitle(title);
    if (!item)
        return;

    ui->categoriesTree->setCurrentItem(item);
}

void ConfigDialog::detailsClicked(const QString& pluginName)
{
    static const QString details = QStringLiteral(
            "<table>"
                "<thead>"
                    "<tr><td colspan=2 align=\"center\"><b>%1</b></td></tr>"
                    "<tr><td colspan=2></td></tr>"
                "</thead>"
                "<tbody>%2</tbody>"
            "</table>");
    static const QString row = QStringLiteral("<tr><td>%1</td><td align=\"right\">%2</td></tr>");
    static const QString hline = QStringLiteral("<tr><td colspan=\"2\"><hr/></td></tr>");

    PluginType* type = PLUGINS->getPluginType(pluginName);
    Q_ASSERT(type != nullptr);

    // Rows
    QStringList rows;
    rows << row.arg(tr("Description:", "plugin details")).arg(PLUGINS->getDescription(pluginName));
    rows << row.arg(tr("Category:", "plugin details")).arg(type->getTitle());
    rows << row.arg(tr("Version:", "plugin details")).arg(PLUGINS->getPrintableVersion(pluginName));
    rows << row.arg(tr("Author:", "plugin details")).arg(PLUGINS->getAuthor(pluginName));
    rows << hline;
    rows << row.arg(tr("Internal name:", "plugin details")).arg(pluginName);
    rows << row.arg(tr("Dependencies:", "plugin details")).arg(PLUGINS->getDependencies(pluginName).join(", "));
    rows << row.arg(tr("Conflicts:", "plugin details")).arg(PLUGINS->getConflicts(pluginName).join(", "));

    // Message
    QString pluginDetails = details.arg(PLUGINS->getTitle(pluginName)).arg(rows.join(""));
    QMessageBox::information(this, tr("Plugin details"), pluginDetails);
}

void ConfigDialog::failedToLoadPlugin(const QString& pluginName)
{
    QTreeWidgetItem* theItem = itemToPluginNameMap.valueByRight(pluginName);
    if (!theItem)
    {
        qWarning() << "Plugin" << pluginName << "failed to load, but it could not be found on the plugins list in ConfigDialog.";
        return;
    }

    theItem->setCheckState(0, Qt::Unchecked);
}

void ConfigDialog::codeFormatterAboutToUnload(Plugin* plugin)
{
    // TODO formatter
//    QString title = plugin->getTitle();
//    if (ui->activeFormatterCombo->currentText() != title)
//        return;

//    if (ui->activeFormatterCombo->count() > 0)
//        ui->activeFormatterCombo->setCurrentIndex(0);

//    ui->activeFormatterCombo->setCurrentIndex(-1);
}

void ConfigDialog::codeFormatterLoaded(Plugin* plugin)
{
    // TODO formatter
//    ui->activeFormatterCombo->addItem(plugin->getTitle(), plugin->getName());
//    configMapper->loadToWidget(CFG_CORE.General.ActiveCodeFormatter, ui->activeFormatterCombo);
}

void ConfigDialog::loadUnloadPlugin(QTreeWidgetItem* item, int column)
{
    if (column != 0)
        return;

    QString pluginName = itemToPluginNameMap.valueByLeft(item);
    if (PLUGINS->isBuiltIn(pluginName))
        return;

    bool wasLoaded = PLUGINS->isLoaded(pluginName);

    if (wasLoaded == (item->checkState(0) == Qt::Checked))
        return;

    if (wasLoaded)
        PLUGINS->unload(pluginName);
    else
        PLUGINS->load(pluginName);

    markModified();
}

void ConfigDialog::pluginAboutToUnload(Plugin* plugin, PluginType* type)
{
    // Update formatters page
    if (type->isForPluginType<CodeFormatterPlugin>())
        codeFormatterAboutToUnload(plugin);

    // Deinit tree item
    QTreeWidgetItem* typeItem = getPluginsCategoryItem(type);
    QTreeWidgetItem* pluginItem = getPluginItem(plugin);
    if (pluginItem)
    {
        typeItem->removeChild(pluginItem);
        pluginToItemMap.remove(plugin);
    }

    // Deinit page
    deinitPluginPage(plugin->getName());

    // Update tree categories
    updatePluginCategoriesVisibility();
}

void ConfigDialog::pluginLoaded(Plugin* plugin, PluginType* type)
{
    if (plugin->getConfigUiForm().isNull())
        return;

    // Update formatters page
    if (type->isForPluginType<CodeFormatterPlugin>())
        codeFormatterLoaded(plugin);

    // Init tree item
    QTreeWidgetItem* typeItem = getPluginsCategoryItem(type);
    QTreeWidgetItem* pluginItem = new QTreeWidgetItem({plugin->getTitle()});
    pluginItem->setStatusTip(0, plugin->getName());
    typeItem->addChild(pluginItem);
    pluginToItemMap[plugin] = pluginItem;

    // Init page
    initPluginPage(plugin->getName(), plugin->getConfigUiForm());

    // Update tree categories
    updatePluginCategoriesVisibility();
}

void ConfigDialog::updatePluginCategoriesVisibility()
{
    QTreeWidgetItem* categories = getPluginsCategoryItem();
    for (int i = 0; i < categories->childCount(); i++)
        updatePluginCategoriesVisibility(categories->child(i));
}

void ConfigDialog::updateBuiltInPluginsVisibility()
{
    bool hideBuiltIn = ui->hideBuiltInPluginsCheck->isChecked();
    QHashIterator<QTreeWidgetItem*,QString> it = itemToPluginNameMap.iterator();
    while (it.hasNext())
    {
        it.next();
        if (PLUGINS->isBuiltIn(it.value()))
            ui->pluginsList->setItemHidden(it.key(), hideBuiltIn);
        else
            ui->pluginsList->setItemHidden(it.key(), false);
    }
}

void ConfigDialog::updatePluginCategoriesVisibility(QTreeWidgetItem* categoryItem)
{
    categoryItem->setHidden(categoryItem->childCount() == 0);
}

QString ConfigDialog::collectLoadedPlugins() const
{
    QStringList loaded;
    QHashIterator<QTreeWidgetItem*,QString> it = itemToPluginNameMap.iterator();
    while (it.hasNext())
    {
        it.next();
        loaded << (it.value() + "=" + ((it.key()->checkState(0) == Qt::Checked) ? "1" : "0"));
    }

    return loaded.join(",");
}

void ConfigDialog::initPageMap()
{
    int pages = ui->stackedWidget->count();
    QWidget* widget;
    for (int i = 0; i < pages; i++)
    {
        widget = ui->stackedWidget->widget(i);
        nameToPage[widget->objectName()] = widget;
    }
}

void ConfigDialog::initInternalCustomConfigWidgets()
{
    QList<CustomConfigWidgetPlugin*> customWidgets;
    customWidgets << new StyleConfigWidget();
//    customWidgets << new ComboDataWidget(&CFG_CORE.General.ActiveCodeFormatter); // TODO formatter
    customWidgets << new ListToStringListHash(&CFG_UI.General.DataEditorsOrder);
    configMapper->setInternalCustomConfigWidgets(customWidgets);
}

void ConfigDialog::initFormatterPlugins()
{
    // TODO formatter
//    SqlFormatterPlugin* formatter = SQLITESTUDIO->getCodeFormatter()->getFormatter();
//    if (!formatter)
//        ui->activeFormatterCombo->setCurrentIndex(-1);
//    else
//        ui->activeFormatterCombo->setCurrentText(formatter->getTitle());

//    connect(ui->activeFormatterCombo, SIGNAL(currentTextChanged(QString)), this, SLOT(activeFormatterChanged(QString)));
//    connect(ui->activeFormatterConfigButton, SIGNAL(clicked()), this, SLOT(activeFormatterConfigurePressed()));

    updateActiveFormatterState();
}

void ConfigDialog::applyStyle(QWidget *widget, QStyle *style)
{
    widget->setStyle(style);
    foreach (QObject* child, widget->children())
    {
        if (!qobject_cast<QWidget*>(child))
            continue;

        applyStyle(qobject_cast<QWidget*>(child), style);
    }
}

QTreeWidgetItem* ConfigDialog::getPluginsCategoryItem() const
{
    QTreeWidgetItem* item = nullptr;
    for (int i = 0; i < ui->categoriesTree->topLevelItemCount(); i++)
    {
         item = ui->categoriesTree->topLevelItem(i);
         if (item->statusTip(0) == ui->pluginsPage->objectName())
             return item;
    }
    Q_ASSERT_X(true, "ConfigDialog", "No Plugins toplevel item in config categories tree!");
    return nullptr;
}

QTreeWidgetItem* ConfigDialog::getPluginsCategoryItem(PluginType* type) const
{
    if (!pluginTypeToItemMap.contains(type))
        return nullptr;

    return pluginTypeToItemMap[type];
}

QTreeWidgetItem* ConfigDialog::getPluginItem(Plugin* plugin) const
{
    if (!pluginToItemMap.contains(plugin))
        return nullptr;

    return pluginToItemMap[plugin];
}

QTreeWidgetItem* ConfigDialog::createPluginsTypeItem(const QString& widgetName, const QString& title) const
{
    if (FORMS->hasWidget(widgetName))
        return new QTreeWidgetItem({title});

    QTreeWidgetItem* pluginsCategoryItem = getPluginsCategoryItem();
    QTreeWidgetItem* item = nullptr;
    for (int i = 0; i < pluginsCategoryItem->childCount(); i++)
    {
        item = pluginsCategoryItem->child(i);
        if (item->statusTip(0) == widgetName)
            return item;
    }
    return nullptr;

}

QTreeWidgetItem* ConfigDialog::getItemByTitle(const QString& title) const
{
    QList<QTreeWidgetItem*> items = ui->categoriesTree->findItems(title, Qt::MatchExactly|Qt::MatchRecursive);
    if (items.size() == 0)
        return nullptr;

    return items.first();
}

void ConfigDialog::switchPage(QTreeWidgetItem *item)
{
    if (isPluginCategoryItem((item)))
    {
        switchPageToPlugin(item);
        return;
    }

    QString name = item->statusTip(0);
    if (!nameToPage.contains(name))
    {
        qWarning() << "Switched page to item" << name << "but there's no such named page defined in ConfigDialog.";
        return;
    }

    ui->stackedWidget->setCurrentWidget(nameToPage[name]);
}

void ConfigDialog::switchPageToPlugin(QTreeWidgetItem *item)
{
    QString pluginName = item->statusTip(0);
    if (!nameToPage.contains(pluginName))
    {
        qCritical() << "No plugin page available for plugin:" << pluginName;
        return;
    }
    ui->stackedWidget->setCurrentWidget(nameToPage[pluginName]);
}

void ConfigDialog::initPlugins()
{
    QTreeWidgetItem *item = getPluginsCategoryItem();

    // Recreate
    QTreeWidgetItem *typeItem;
    foreach (PluginType* pluginType, PLUGINS->getPluginTypes())
    {
        typeItem = createPluginsTypeItem(pluginType->getConfigUiForm(), pluginType->getTitle());
        if (!typeItem)
            continue;

        item->addChild(typeItem);
        pluginTypeToItemMap[pluginType] = typeItem;

        foreach (Plugin* plugin, pluginType->getLoadedPlugins())
            pluginLoaded(plugin, pluginType);
    }

    updatePluginCategoriesVisibility();

    connect(PLUGINS, SIGNAL(loaded(Plugin*,PluginType*)), this, SLOT(pluginLoaded(Plugin*,PluginType*)));
    connect(PLUGINS, SIGNAL(aboutToUnload(Plugin*,PluginType*)), this, SLOT(pluginAboutToUnload(Plugin*,PluginType*)));
}

void ConfigDialog::initPluginsPage()
{
    QTreeWidgetItem* category;
    QTreeWidgetItem* item;
    QFont font;
    QModelIndex categoryIndex;
    QModelIndex itemIndex;
    int itemRow;
    int categoryRow;
    bool builtIn;
    QLabel* detailsLabel;
    QString title;
    QSize itemSize;
    QStringList pluginNames;

    // Font and metrics
    item = new QTreeWidgetItem({""});
    font = item->font(0);

    QFontMetrics fm(font);
    itemSize = QSize(-1, (fm.ascent() + fm.descent() + 4));

    delete item;

    // Creating...
    ui->pluginsPageInfoIcon->setPixmap(ICONS.INFO_BALLOON);

    ui->pluginsList->header()->setSectionsMovable(false);
    ui->pluginsList->header()->setSectionResizeMode(0, QHeaderView::Stretch);

    QBrush categoryBg = ui->pluginsList->palette().button();
    QBrush categoryFg = ui->pluginsList->palette().buttonText();

    connect(ui->pluginsList, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(loadUnloadPlugin(QTreeWidgetItem*,int)));
    connect(PLUGINS, SIGNAL(failedToLoad(QString)), this, SLOT(failedToLoadPlugin(QString)));

    categoryRow = 0;
    QList<PluginType*> pluginTypes = PLUGINS->getPluginTypes();
    qSort(pluginTypes.begin(), pluginTypes.end(), PluginType::nameLessThan);
    foreach (PluginType* pluginType, pluginTypes)
    {
        category = new QTreeWidgetItem({pluginType->getTitle()});
        font.setItalic(false);
        font.setBold(true);
        category->setFont(0, font);
        for (int i = 0; i < 2; i++)
        {
            category->setBackground(i, categoryBg);
            category->setForeground(i, categoryFg);
        }
        category->setSizeHint(0, itemSize);
        ui->pluginsList->addTopLevelItem(category);

        categoryIndex = ui->pluginsList->model()->index(categoryRow, 0);
        categoryRow++;

        itemRow = 0;
        pluginNames = pluginType->getAllPluginNames();
        qSort(pluginNames);
        foreach (const QString& pluginName, pluginNames)
        {
            builtIn = PLUGINS->isBuiltIn(pluginName);
            title = PLUGINS->getTitle(pluginName);
            if (builtIn)
                title += tr(" (built-in)", "plugins manager in configuration dialog");

            item = new QTreeWidgetItem({title});
            item->setCheckState(0, PLUGINS->isLoaded(pluginName) ? Qt::Checked : Qt::Unchecked);
            item->setSizeHint(0, itemSize);
            if (builtIn)
                item->setDisabled(true);

            category->addChild(item);

            itemToPluginNameMap.insert(item, pluginName);

            // Details button
            detailsLabel = new QLabel(QString("<a href='%1'>%2</a> ").arg(pluginName).arg(tr("Details")), ui->pluginsList);
            detailsLabel->setAlignment(Qt::AlignRight);
            itemIndex = ui->pluginsList->model()->index(itemRow, 1, categoryIndex);
            ui->pluginsList->setIndexWidget(itemIndex, detailsLabel);

            connect(detailsLabel, SIGNAL(linkActivated(QString)), this, SLOT(detailsClicked(QString)));

            itemRow++;
        }

        if (itemRow == 0)
        {
            item = new QTreeWidgetItem({tr("No plugins in this category.")});
            item->setDisabled(true);
            item->setSizeHint(0, itemSize);

            font.setItalic(true);
            font.setBold(false);
            item->setFont(0, font);

            category->addChild(item);
        }

        category->setExpanded(true);
    }
}

void ConfigDialog::initPluginPage(const QString& pluginName, const QString& formName)
{
    QWidget* widget = FORMS->createWidget(formName);
    if (!widget)
    {
        qWarning() << "Could not load plugin UI file" << formName << "for plugin:" << pluginName;
        return;
    }

    nameToPage[pluginName] = widget;
    ui->stackedWidget->addWidget(widget);
    configMapper->loadToWidget(widget);
}

void ConfigDialog::deinitPluginPage(const QString& pluginName)
{
    if (!nameToPage.contains(pluginName))
        return;

    QWidget* widget = nameToPage[pluginName];
    nameToPage.remove(pluginName);
    ui->stackedWidget->removeWidget(widget);
    delete widget;
}

void ConfigDialog::initDataEditors()
{
    ui->dataEditorsAvailableList->setSpacing(1);

    QHash<QString,QVariant> editorsOrder = CFG_UI.General.DataEditorsOrder.get();
    QSet<QString> dataTypeSet = editorsOrder.keys().toSet();
    dataTypeSet += DataType::getAllNames().toSet();
    QStringList dataTypeList = dataTypeSet.toList();
    qSort(dataTypeList);

    QListWidgetItem* item;
    for (const QString& type : dataTypeList)
    {
        item = new QListWidgetItem(type);
        if (!DataType::getAllNames().contains(type))
            item->setFlags(item->flags()|Qt::ItemIsEditable);

        ui->dataEditorsTypesList->addItem(item);
    }

    QAction* act = new QAction(ICONS.INSERT_DATATYPE, tr("Add new data type"), ui->dataEditorsTypesToolbar);
    connect(act, SIGNAL(triggered()), this, SLOT(addDataType()));
    ui->dataEditorsTypesToolbar->addAction(act);

    dataEditRenameAction = new QAction(ICONS.RENAME_DATATYPE, tr("Rename selected data type"), ui->dataEditorsTypesToolbar);
    connect(dataEditRenameAction, SIGNAL(triggered()), this, SLOT(renameDataType()));
    ui->dataEditorsTypesToolbar->addAction(dataEditRenameAction);

    dataEditDeleteAction = new QAction(ICONS.DELETE_DATATYPE, tr("Delete selected data type"), ui->dataEditorsTypesToolbar);
    connect(dataEditDeleteAction, SIGNAL(triggered()), this, SLOT(delDataType()));
    ui->dataEditorsTypesToolbar->addAction(dataEditDeleteAction);

    act = new QAction(ICONS.HELP, tr("Help for configuring data type editors"), ui->dataEditorsTypesToolbar);
    connect(act, SIGNAL(triggered()), this, SLOT(dataTypesHelp()));
    ui->dataEditorsTypesToolbar->addAction(act);

    connect(ui->dataEditorsTypesList->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(updateDataTypeEditors()));
    connect(ui->dataEditorsTypesList->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(updateDataTypeListState()));
    connect(ui->dataEditorsTypesList, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(dataEditorItemEdited(QListWidgetItem*)));
    connect(ui->dataEditorsAvailableList, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(dataEditorAvailableChanged(QListWidgetItem*)));
    connect(ui->dataEditorsSelectedTabs->tabBar(), SIGNAL(tabMoved(int,int)), this, SLOT(dataEditorTabsOrderChanged(int,int)));

    ui->dataEditorsTypesList->setCurrentRow(0, QItemSelectionModel::Clear|QItemSelectionModel::SelectCurrent);
    updateDataTypeListState();
}

bool ConfigDialog::isPluginCategoryItem(QTreeWidgetItem *item) const
{
    return item->parent() && item->parent()->parent() && item->parent()->parent() == getPluginsCategoryItem();
}

void ConfigDialog::updateStylePreview()
{
    ui->previewWidget->parentWidget()->layout()->removeWidget(ui->previewWidget);
    ui->previewTabs->currentWidget()->layout()->addWidget(ui->previewWidget);
    ui->previewWidget->setEnabled(ui->previewTabs->currentIndex() == 0);

    QStyle* previousStyle = previewStyle;
    previewStyle = QStyleFactory::create(ui->activeStyleCombo->currentText());
    if (!previewStyle)
    {
        qWarning() << "Could not create style:" << ui->activeStyleCombo->currentText();
        return;
    }

    applyStyle(ui->activeStylePreviewGroup, previewStyle);

    if (previousStyle)
        delete previousStyle;
}

void ConfigDialog::apply()
{
    if (modifiedFlag)
        save();

    setModified(false);
}

void ConfigDialog::accept()
{
    apply();
    QDialog::accept();
}
