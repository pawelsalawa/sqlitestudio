#ifndef CELLRENDERERTABLE_H
#define CELLRENDERERTABLE_H

#include "customconfigwidgetplugin.h"
#include "plugins/genericplugin.h"
#include "guiSQLiteStudio_global.h"
#include <QTableWidget>

class ComboNoWheelFilter;
class QComboBox;

class GUI_API_EXPORT CellRendererTable : public QTableWidget
{
        Q_OBJECT

    public:
        explicit CellRendererTable(QWidget *parent = nullptr);

        void addRendererDataType(const QString& typeStr, int row);

    private:
        void defineDefaultStyleForRendererCombo(QComboBox* combo);
        void defineAltStyleForRendererCombo(QComboBox* combo, int idx);
        void applyRendererComboStyle(QComboBox* combo, int idx);

        ComboNoWheelFilter* comboNoWheelFilter = nullptr;

    private slots:
        void handlePluginUnload(Plugin* plugin, PluginType* type);
        void handlePluginLoaded(Plugin* plugin, PluginType* type);

    signals:
        void modified();
};

class GUI_API_EXPORT CellRendererTableToHash : public GenericPlugin, public CustomConfigWidgetPlugin
{
    public:
        CellRendererTableToHash();
        bool isConfigForWidget(CfgEntry* key, QWidget* widget);
        void applyConfigToWidget(CfgEntry* key, QWidget* widget, const QVariant& value);
        QVariant getWidgetConfigValue(QWidget* widget, bool& ok);
        const char* getModifiedNotifier() const;
        QString getFilterString(QWidget* widget) const;
};

#endif // CELLRENDERERTABLE_H
