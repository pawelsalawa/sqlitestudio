#ifndef CONFIGMAPPER_H
#define CONFIGMAPPER_H

#include "common/bihash.h"
#include <QObject>

class CfgMain;
class CfgEntry;
class CustomConfigWidgetPlugin;
class ConfigComboBox;

class ConfigMapper : public QObject
{
        Q_OBJECT

    public:
        explicit ConfigMapper(CfgMain* cfgMainList);
        ConfigMapper(const QList<CfgMain*> cfgMainList);

        void loadToWidget(QWidget* topLevelWidget);
        void loadToWidget(CfgEntry* config, QWidget* widget);
        void saveFromWidget(QWidget* widget);
        void setInternalCustomConfigWidgets(const QList<CustomConfigWidgetPlugin*>& value);

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

        QWidget* getBindWidgetForConfig(CfgEntry* key) const;
        CfgEntry* getBindConfigForWidget(QWidget* widget) const;

    private:
        void applyConfigToWidget(QWidget *widget, const QHash<QString, CfgEntry*>& allConfigEntries, const QHash<QString, QVariant> &config);
        void applyConfigToWidget(QWidget *widget, CfgEntry* cfgEntry, const QVariant& configValue);
        void handleSpecialWidgets(QWidget *widget, const QHash<QString, CfgEntry*>& allConfigEntries);
        void handleConfigComboBox(QWidget *widget, const QHash<QString, CfgEntry*>& allConfigEntries);
        void connectCommonNotifierToWidget(QWidget *widget, CfgEntry* key);
        bool connectCustomNotifierToWidget(QWidget *widget, CfgEntry* cfgEntry);
        void applyCommonConfigToWidget(QWidget *widget, const QVariant& value, CfgEntry* cfgEntry);
        bool applyCustomConfigToWidget(CfgEntry* key, QWidget *widget, const QVariant& value);
        void saveWidget(QWidget* widget, const QHash<QString, CfgEntry*>& allConfigEntries);
        void saveFromWidget(QWidget* widget, CfgEntry* cfgEntry);
        void saveCommonConfigFromWidget(QWidget *widget, CfgEntry* key);
        bool saveCustomConfigFromWidget(QWidget *widget, CfgEntry* key);
        CfgEntry* getConfigEntry(QWidget* widget, const QHash<QString, CfgEntry*>& allConfigEntries);
        CfgEntry* getEntryForProperty(QWidget* widget, const char* propertyName, const QHash<QString, CfgEntry*>& allConfigEntries);
        QHash<QString,CfgEntry*> getAllConfigEntries();
        QList<QWidget*> getAllConfigWidgets(QWidget* parent);
        bool isPersistant() const;

        QList<CfgMain*> cfgMainList;
        QList<CustomConfigWidgetPlugin*> internalCustomConfigWidgets;
        bool realTimeUpdates = false;
        QHash<QWidget*,CfgEntry*> widgetToConfigEntry;
        QHash<CfgEntry*,QWidget*> configEntryToWidgets;
        QHash<CfgEntry*,QWidget*> specialConfigEntryToWidgets;
        bool updatingEntry = false;

    private slots:
        void handleModified();
        void entryChanged(const QVariant& newValue);
        void updateConfigComboModel(const QVariant& value);

    signals:
        void modified();
};

#endif // CONFIGMAPPER_H
