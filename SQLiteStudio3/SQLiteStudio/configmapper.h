#ifndef CONFIGMAPPER_H
#define CONFIGMAPPER_H

#include <QObject>

class CfgMain;
class CfgEntry;
class CustomConfigWidgetPlugin;

class ConfigMapper : public QObject
{
        Q_OBJECT

    public:
        explicit ConfigMapper(CfgMain* cfgMainList);
        ConfigMapper(const QList<CfgMain*> cfgMainList);

        void loadToWidget(QWidget* widget);
        void loadToWidget(CfgEntry* config, QWidget* widget);
        void saveFromWidget(QWidget* widget);
        void setInternalCustomConfigWidgets(const QList<CustomConfigWidgetPlugin*>& value);

    private:
        void applyConfigToWidget(QWidget *widget, const QHash<QString, CfgEntry *> &allConfigEntries, const QHash<QString, QVariant> &config);
        void applyCommonConfigToWidget(QWidget *widget, const QVariant& value);
        bool applyCustomConfigToWidget(CfgEntry* key, QWidget *widget, const QVariant& value);
        void saveWidget(QWidget* widget, const QHash<QString, CfgEntry*>& allConfigEntries);
        void saveCommonConfigFromWidget(QWidget *widget, CfgEntry* key);
        bool saveCustomConfigFromWidget(QWidget *widget, CfgEntry* key);
        CfgEntry* getConfigEntry(QWidget* widget, const QHash<QString, CfgEntry*>& allConfigEntries);
        QHash<QString,CfgEntry*> getAllConfigEntries();
        QList<QWidget*> getAllConfigWidgets(QWidget* parent);
        bool isPersistant() const;

        QList<CfgMain*> cfgMainList;
        QList<CustomConfigWidgetPlugin*> internalCustomConfigWidgets;

    signals:
        void modified();
};

#endif // CONFIGMAPPER_H
