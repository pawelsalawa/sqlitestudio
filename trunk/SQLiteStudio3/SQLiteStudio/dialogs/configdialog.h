#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include "config_builder.h"
#include "datatype.h"
#include "common/bihash.h"
#include <QDialog>

namespace Ui {
    class ConfigDialog;
}

class QListWidgetItem;
class QTreeWidgetItem;
class CustomConfigWidgetPlugin;
class QSignalMapper;
class Plugin;
class PluginType;
class QComboBox;
class QTreeWidget;
class QListWidget;
class QTableWidget;
class ConfigMapper;
class MultiEditorWidgetPlugin;

class ConfigDialog : public QDialog
{
    Q_OBJECT

    public:
        explicit ConfigDialog(QWidget *parent = 0);
        ~ConfigDialog();

        void configureDataEditors(const QString& dataTypeString);

        static QString getFilterString(QWidget* widget);
        static QString getFilterString(QComboBox* widget);
        static QString getFilterString(QTreeWidget* widget);
        static QString getFilterString(QListWidget* widget);
        static QString getFilterString(QTableWidget* widget);

    private:
        void init();
        void load();
        void initPageMap();
        void initInternalCustomConfigWidgets();
        void initFormatterPlugins();
        void initPlugins();
        void initPluginsPage();
        void initPluginPage(const QString& pluginName, const QString& formName);
        void deinitPluginPage(const QString& pluginName);
        void initDataEditors();
        void applyStyle(QWidget* widget, QStyle* style);
        QTreeWidgetItem* getPluginsCategoryItem() const;
        QTreeWidgetItem* getPluginsCategoryItem(PluginType* type) const;
        QTreeWidgetItem* getPluginItem(Plugin* plugin) const;
        QTreeWidgetItem* createPluginsTypeItem(const QString& widgetName, const QString& title) const;
        QTreeWidgetItem* getItemByTitle(const QString& title) const;
        void switchPageToPlugin(QTreeWidgetItem* item);
        bool isPluginCategoryItem(QTreeWidgetItem *item) const;
        void sqlFormatterAboutToUnload(Plugin* plugin);
        void sqlFormatterLoaded(Plugin* plugin);
        void updatePluginCategoriesVisibility(QTreeWidgetItem* categoryItem);
        QString collectLoadedPlugins() const;
        QHash<QWidget*,QTreeWidgetItem*> buildPageToCategoryItemMap() const;
        QList<QTreeWidgetItem*> getAllCategoryItems() const;
        QList<MultiEditorWidgetPlugin*> getDefaultEditorsForType(DataType::Enum dataType);
        QList<MultiEditorWidgetPlugin*> updateCustomDataTypeEditors(const QStringList& editorsOrder);
        QList<MultiEditorWidgetPlugin*> updateDefaultDataTypeEditors(DataType::Enum typeEnum);
        void addDataTypeEditor(const QString& pluginName);
        void addDataTypeEditor(MultiEditorWidgetPlugin* plugin);
        void removeDataTypeEditor(QListWidgetItem* item, const QString& pluginName);
        void removeDataTypeEditor(int idx);
        void transformDataTypeEditorsToCustomList(QListWidgetItem* typeItem);
        QStringList getPluginNamesFromDataTypeItem(QListWidgetItem* typeItem);
        void setPluginNamesForDataTypeItem(QListWidgetItem* typeItem, const QStringList& pluginNames);
        void addDataType(const QString& typeStr);

        Ui::ConfigDialog *ui;
        QStyle* previewStyle = nullptr;
        QHash<QString,QWidget*> nameToPage;
        BiHash<QTreeWidgetItem*,QString> itemToPluginNameMap;
        QHash<PluginType*,QTreeWidgetItem*> pluginTypeToItemMap;
        QHash<Plugin*,QTreeWidgetItem*> pluginToItemMap;
        ConfigMapper* configMapper = nullptr;
        QAction* dataEditRenameAction = nullptr;
        QAction* dataEditDeleteAction = nullptr;
        bool updatingDataEditorItem = false;

    private slots:
        void pageSwitched();
        void updateDataTypeEditors();
        void updateDataTypeListState();
        void dataEditorItemEdited(QListWidgetItem* item);
        void dataEditorAvailableChanged(QListWidgetItem* item);
        void dataEditorTabsOrderChanged(int from, int to);
        void addDataType();
        void renameDataType();
        void delDataType();
        void dataTypesHelp();
        void switchPage(QTreeWidgetItem* item);
        void updateStylePreview();
        void apply();
        void save();
        void modified();
        void setModified(bool modified);
        void applyFilter(const QString& filter);
        void updateActiveFormatterState();
        void activeFormatterChanged(const QString& title);
        void activeFormatterConfigurePressed();
        void detailsClicked(const QString& pluginName);
        void failedToLoadPlugin(const QString& pluginName);
        void loadUnloadPlugin(QTreeWidgetItem* item, int column);
        void pluginAboutToUnload(Plugin* plugin, PluginType* type);
        void pluginLoaded(Plugin* plugin, PluginType* type);
        void updatePluginCategoriesVisibility();
        void updateBuiltInPluginsVisibility();

    public slots:
        void accept();
};

#endif // CONFIGDIALOG_H
