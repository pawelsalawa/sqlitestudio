#ifndef CONFIGMAPPER_H
#define CONFIGMAPPER_H

#include "common/bihash.h"
#include "guiSQLiteStudio_global.h"
#include <QObject>
#include <QSet>

class CfgMain;
class CfgEntry;
class CustomConfigWidgetPlugin;
class ConfigComboBox;

class GUI_API_EXPORT ConfigMapper : public QObject
{
        Q_OBJECT

    public:
        explicit ConfigMapper(CfgMain* cfgMainList);
        ConfigMapper(const QList<CfgMain*> cfgMainList);

        void loadToWidget(QWidget* topLevelWidget);
        void loadToWidget(CfgEntry* config, QWidget* widget);
        void saveFromWidget(QWidget* widget, bool noTransaction = false);
        void setInternalCustomConfigWidgets(const QList<CustomConfigWidgetPlugin*>& value);
        bool isPersistant() const;
        void applyConfigDefaultValueToWidget(QWidget *widget);
        QList<QWidget*> getAllConfigWidgets(QWidget* parent);
        CfgEntry* getConfigForWidget(QWidget* widget);

        /**
         * @brief Scans widget and binds widgets to proper config objects.
         * @param topLevelWidget Toplevel widget that contains all required configuration widgets.
         *
         * It goes through all widgets in given top level widget and looks for statusText property
         * that matches any of config entries in CfgMain instance(s) passed in constructor.
         *
         * For each matched pair (QWidget::statusText() to CfgEntry::getFullKey()) it remembers
         * pair of QWidget and CfgEntry and from now on when CfgEntry is modified, the QWidget
         * gets updated with the value and vice versa - when QWidget gets new input, then CfgEntry
         * is updated with that value. All these updates are done in real time.
         *
         * @note Binding mechanism can be used only against CfgMain that is not persistable.
         * If you try to bind CfgMain that is persistable, it just won't work and it is intended
         * that way, because having a QWidget bind to a persistable CfgEntry would cause lots
         * of database updates to store changing value (like from QLineEdit). For persistable please
         * use loadToWidget() and saveFromWidget() methods, as you can call them when necessary.
         */
        void bindToConfig(QWidget* topLevelWidget);

        /**
         * @brief Releases any bingins against widgets.
         *
         * This simply revokes what was done with bindToConfig().
         */
        void unbindFromConfig();
        void removeMainCfgEntry(CfgMain* cfgMain);

        QWidget* getBindWidgetForConfig(CfgEntry* key) const;
        CfgEntry* getBindConfigForWidget(QWidget* widget) const;

        QList<QWidget *> getExtraWidgets() const;

        /**
         * @brief Sets list of extra widgets to load/save values to/from.
         * @param value List of widgets.
         *
         * Extra widgets list can be provided to the mapper if it will be impossible to find those widgets
         * just by looking for childrens of top widget recurrently. This is for example the case
         * when widgets are embedded in QAbstractItemView::setIndexWidget(). Such widgets have to be
         * provided as list of extra widgets to the mapper, so the mapper handles them when loading/saving
         * values.
         */
        void setExtraWidgets(const QList<QWidget *> &value);
        void addExtraWidget(QWidget* w);
        void addExtraWidgets(const QList<QWidget*>& list);
        void clearExtraWidgets();
        void ignoreWidget(QWidget* w);
        void removeIgnoredWidget(QWidget* w);
        void saveFromWidget(QWidget* widget, CfgEntry* cfgEntry);

    private:
        void applyConfigToWidget(QWidget *widget, const QHash<QString, CfgEntry*>& allConfigEntries, const QHash<QString, QVariant> &config);
        void applyConfigToWidget(QWidget *widget, CfgEntry* cfgEntry, const QVariant& configValue);
        void handleSpecialWidgets(QWidget *widget, const QHash<QString, CfgEntry*>& allConfigEntries);
        void handleConfigComboBox(QWidget *widget, const QHash<QString, CfgEntry*>& allConfigEntries);
        void handleFileEdit(QWidget* widget, const QHash<QString, CfgEntry*>& allConfigEntries);
        void connectCommonNotifierToWidget(QWidget *widget, CfgEntry* key);
        bool connectCustomNotifierToWidget(QWidget *widget, CfgEntry* cfgEntry);
        void applyCommonConfigToWidget(QWidget *widget, const QVariant& value, CfgEntry* cfgEntry);
        bool applyCustomConfigToWidget(CfgEntry* key, QWidget *widget, const QVariant& value);
        void saveWidget(QWidget* widget, const QHash<QString, CfgEntry*>& allConfigEntries);
        void saveCommonConfigFromWidget(QWidget *widget, CfgEntry* key);
        bool saveCustomConfigFromWidget(QWidget *widget, CfgEntry* key);
        QVariant getCommonConfigValueFromWidget(QWidget *widget, CfgEntry* key, bool& ok);
        QVariant getCustomConfigValueFromWidget(QWidget *widget, CfgEntry* key, bool& ok);
        QVariant getConfigValueFromWidget(QWidget *widget, CfgEntry* key);
        QVariant getConfigValueFromWidget(QWidget *widget);
        CfgEntry* getConfigEntry(QWidget* widget, const QHash<QString, CfgEntry*>& allConfigEntries);
        CfgEntry* getEntryForProperty(QWidget* widget, const char* propertyName, const QHash<QString, CfgEntry*>& allConfigEntries);
        QHash<QString,CfgEntry*> getAllConfigEntries();
        void handleDependencySettings(QWidget* widget);
        void handleBoolDependencySettings(const QString& boolDependency, QWidget* widget);
        void handleDependencyChange(QWidget* widget);
        bool handleBoolDependencyChange(QWidget* widget);
        QString getConfigFullKeyForWidget(QWidget* widget);

        QWidget* configDialogWidget = nullptr;
        QList<CfgMain*> cfgMainList;
        QList<CustomConfigWidgetPlugin*> internalCustomConfigWidgets;
        bool realTimeUpdates = false;
        QHash<QWidget*,CfgEntry*> widgetToConfigEntry;
        QMultiHash<CfgEntry*,QWidget*> configEntryToWidgets;
        QMultiHash<CfgEntry*,QWidget*> specialConfigEntryToWidgets;
        bool updatingEntry = false;
        QList<QWidget*> extraWidgets;
        QList<QWidget*> widgetsToIgnore; // main mapper will ignore plugin's forms, they have their own mappers
        QHash<QString, CfgEntry*> allEntries;
        QHash<QWidget*, QWidget*> boolDependencyToDependingWidget;

        static constexpr const char* CFG_MODEL_PROPERTY = "cfg";
        static constexpr const char* CFG_BOOL_DEPENDENCY_PROPERTY = "boolDependency";
        static constexpr const char* CFG_NOTIFY_PROPERTY = "notify";
        static constexpr const char* CFG_PREVIEW_PROPERTY = "preview";

    private slots:
        void handleModified();
        void handleCustomModified();
        void entryChanged(const QVariant& newValue);
        void uiConfigEntryChanged();
        void updateConfigComboModel(const QVariant& value);
        void updateFileEditChoicesModel(const QVariant& value);
        void notifiableConfigKeyChanged();

    signals:
        void modified(QWidget* widget);
        void notifyEnabledWidgetModified(QWidget* widget, CfgEntry* key, const QVariant& value);
};

#endif // CONFIGMAPPER_H
