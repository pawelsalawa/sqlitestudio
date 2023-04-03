#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include "config_builder.h"
#include "datatype.h"
#include "common/bihash.h"
#include "guiSQLiteStudio_global.h"
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
class QToolButton;
class QTreeWidget;
class QListWidget;
class QTableWidget;
class ConfigMapper;
class MultiEditorWidgetPlugin;
class ConfigNotifiablePlugin;
class UiConfiguredPlugin;
class SyntaxHighlighterPlugin;
class QPlainTextEdit;
class QSyntaxHighlighter;
class SqlEditor;

#define CFG_UI_NAME "Ui"
#define CFG_COLORS_NAME "Colors"

class GUI_API_EXPORT ConfigDialog : public QDialog
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

    protected:
        void showEvent(QShowEvent* event);

    private:
        void init();
        void load();
        void initPageMap();
        void initInternalCustomConfigWidgets();
        void initFormatterPlugins();
        void initPlugins();
        void initPluginsPage();
        bool initPluginPage(Plugin* plugin, bool skipConfigLoading);
        void deinitPluginPage(Plugin* pluginName);
        void initDataEditors();
        void initShortcuts();
        void initShortcuts(CfgCategory* cfgCategory);
        void initLangs();
        void initTooltips();
        void initColors();
        void updateColorsAfterLoad();
        void toggleColorButtonState(CfgEntry* colorCheckEntry);
        void applyStyle(QWidget* widget, QStyle* style);
        QTreeWidgetItem* getPluginsCategoryItem() const;
        QTreeWidgetItem* getPluginsCategoryItem(PluginType* type) const;
        QTreeWidgetItem* getPluginItem(Plugin* plugin) const;
        QTreeWidgetItem* createPluginsTypeItem(const QString& widgetName, const QString& title) const;
        QTreeWidgetItem* getItemByTitle(const QString& title) const;
        void switchPageToPlugin(QTreeWidgetItem* item);
        bool isPluginCategoryItem(QTreeWidgetItem *item) const;
        void codeFormatterUnloaded();
        void codeFormatterLoaded();
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
        QStringList getPluginNamesFromDataTypeItem(QListWidgetItem* typeItem, bool* exists = nullptr);
        void setPluginNamesForDataTypeItem(QListWidgetItem* typeItem, const QStringList& pluginNames);
        void addDataType(const QString& typeStr);
        void rollbackPluginConfigs();
        void rollbackColorsConfig();
        void commitPluginConfigs();
        void commitColorsConfig();
        void connectMapperSignals(ConfigMapper* mapper);
        QList<CfgMain*> getShortcutsCfgMains() const;
        QList<CfgCategory*> getShortcutsCfgCategories() const;
        void refreshColorsInSyntaxHighlighters();
        void colorChanged();
        QList<QWidget*> prepareCodeSyntaxColorsForStyle();
        void adjustSyntaxColorsForStyle(QList<QWidget*>& unmodifiedColors);
        void highlighterPluginLoaded(SyntaxHighlighterPlugin* plugin);
        void highlighterPluginUnloaded(SyntaxHighlighterPlugin* plugin);

        Ui::ConfigDialog *ui = nullptr;
        QStyle* previewStyle = nullptr;
        QHash<QString,QWidget*> nameToPage;
        BiHash<QTreeWidgetItem*,QString> pluginListItemToPluginNameMap;
        QHash<PluginType*,QTreeWidgetItem*> pluginTypeToItemMap;
        QHash<Plugin*,QTreeWidgetItem*> pluginToItemMap;
        QHash<QString,QComboBox*> formatterLangToPluginComboMap;
        QHash<QString,QToolButton*> formatterLangToConfigButtonMap;
        ConfigMapper* configMapper = nullptr;
        QHash<UiConfiguredPlugin*,ConfigMapper*> pluginConfigMappers;
        QAction* dataEditRenameAction = nullptr;
        QAction* dataEditDeleteAction = nullptr;
        bool updatingDataEditorItem = false;
        bool modifiedFlag = false;
        QList<ConfigNotifiablePlugin*> notifiablePlugins;
        bool requiresSchemasRefresh = false;
        QList<QPlainTextEdit*> colorPreviewEditors;
        SqlEditor* codePreviewSqlEditor = nullptr;
        QList<QSyntaxHighlighter*> colorPreviewHighlighters;
        BiHash<QPlainTextEdit*, SyntaxHighlighterPlugin*> highlightingPluginForPreviewEditor;
        bool resettingColors = false;

    private slots:
        void refreshFormattersPage();
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
        void storeSelectedFormatters();
        void markModified();
        void setModified(bool modified);
        void updateModified();
        void applyFilter(const QString& filter);
        void updateActiveFormatterState();
        void configureFormatter(const QString& pluginTitle);
        void activeFormatterChanged();
        void detailsClicked(const QString& pluginName);
        void failedToLoadPlugin(const QString& pluginName);
        void loadUnloadPlugin(QTreeWidgetItem* item, int column);
        void pluginAboutToUnload(Plugin* plugin, PluginType* type);
        void pluginLoaded(Plugin* plugin, PluginType* type, bool skipConfigLoading = false);
        void pluginUnloaded(const QString& pluginName, PluginType* type);
        void updatePluginCategoriesVisibility();
        void updateBuiltInPluginsVisibility();
        void applyShortcutsFilter(const QString& filter);
        void markRequiresSchemasRefresh();
        void notifyPluginsAboutModification(QWidget*, CfgEntry* key, const QVariant& value);
        void resetCodeSyntaxColors();

    public slots:
        void accept();
};

#endif // CONFIGDIALOG_H
