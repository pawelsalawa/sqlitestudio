#include "cellrenderertable.h"
#include "uiconfig.h"
#include "services/pluginmanager.h"
#include "datagrid/cellrendererplugin.h"
#include "dialogs/configdialog.h"
#include "common/combonowheelfilter.h"
#include <QComboBox>

CellRendererTable::CellRendererTable(QWidget* parent) :
    QTableWidget(parent)
{
    comboNoWheelFilter = new ComboNoWheelFilter(this);
    connect(PLUGINS, SIGNAL(loaded(Plugin*,PluginType*)), this, SLOT(handlePluginLoaded(Plugin*,PluginType*)));
    connect(PLUGINS, SIGNAL(aboutToUnload(Plugin*,PluginType*)), this, SLOT(handlePluginUnload(Plugin*,PluginType*)));
    connect(this, SIGNAL(itemChanged(QTableWidgetItem*)), this, SIGNAL(modified()));
}

void CellRendererTable::addRendererDataType(const QString& typeStr, int row)
{
    QTableWidgetItem* item = new QTableWidgetItem(typeStr);
    if (DataType::getAllNames().contains(typeStr))
        item->setFlags(item->flags()&~Qt::ItemIsEditable);
    else
        item->setFlags(item->flags()|Qt::ItemIsEditable);

    setItem(row, 0, item);

    QComboBox* combo = new QComboBox(this);
    combo->installEventFilter(comboNoWheelFilter);
    combo->setEditable(false);
    combo->setProperty("type", typeStr);

    combo->addItem(tr("Default"));
    defineDefaultStyleForRendererCombo(combo);
    for (CellRendererPlugin* plugin : PLUGINS->getLoadedPlugins<CellRendererPlugin>())
    {
        combo->addItem(plugin->getRendererName(), plugin->getName());
        defineAltStyleForRendererCombo(combo, combo->count() - 1);
    }
    applyRendererComboStyle(combo, combo->currentIndex());

    connect(combo, &QComboBox::currentIndexChanged, this, [this, combo, item](int index)
    {
        applyRendererComboStyle(combo, index);
        emit modified();
    });

    setCellWidget(row, 1, combo);
}

void CellRendererTable::defineDefaultStyleForRendererCombo(QComboBox* combo)
{
    QFont font = combo->font();
    font.setItalic(true);
    combo->setItemData(0, font, Qt::FontRole);
}

void CellRendererTable::defineAltStyleForRendererCombo(QComboBox* combo, int idx)
{
    QFont font = combo->font();
    font.setBold(true);
    combo->setItemData(idx, font, Qt::FontRole);
}

void CellRendererTable::applyRendererComboStyle(QComboBox* combo, int idx)
{
    if (idx < 0 || idx >= combo->count())
        return;

    combo->setFont(combo->itemData(idx, Qt::FontRole).value<QFont>());
}

void CellRendererTable::handlePluginUnload(Plugin* plugin, PluginType* type)
{
    Q_UNUSED(type);
    CellRendererPlugin* rendererPlugin = dynamic_cast<CellRendererPlugin*>(plugin);
    if (!rendererPlugin)
        return;

    for (int row = 0, total = rowCount(); row < total; row++)
    {
        QComboBox* combo = dynamic_cast<QComboBox*>(cellWidget(row, 1));
        if (!combo)
            continue;

        QString pluginName = combo->currentData().toString();
        if (pluginName == rendererPlugin->getName())
        {
            combo->removeItem(combo->currentIndex());
            combo->setCurrentIndex(0);
        }
    }
}

void CellRendererTable::handlePluginLoaded(Plugin* plugin, PluginType* type)
{
    Q_UNUSED(type);
    CellRendererPlugin* rendererPlugin = dynamic_cast<CellRendererPlugin*>(plugin);
    if (!rendererPlugin)
        return;

    for (int row = 0, total = rowCount(); row < total; row++)
    {
        QComboBox* combo = dynamic_cast<QComboBox*>(cellWidget(row, 1));
        if (!combo)
            continue;

        combo->addItem(rendererPlugin->getRendererName(), rendererPlugin->getName());
        defineAltStyleForRendererCombo(combo, combo->count() - 1);
    }
}

CellRendererTableToHash::CellRendererTableToHash()
{
}

bool CellRendererTableToHash::isConfigForWidget(CfgEntry* key, QWidget* widget)
{
    Q_UNUSED(widget);
    return (CFG_UI.General.DataRenderers == key);
}

void CellRendererTableToHash::applyConfigToWidget(CfgEntry* key, QWidget* widget, const QVariant& value)
{
    Q_UNUSED(key);

    CellRendererTable* table = dynamic_cast<CellRendererTable*>(widget);
    table->setRowCount(0);

    QHash<QString, QVariant> renderers = value.toHash();
    QStringList dataTypeList = renderers.keys();
    sSort(dataTypeList);
    table->setRowCount(dataTypeList.size());
    int row = 0;
    for (QString& type : dataTypeList)
    {
        table->addRendererDataType(type, row);

        QComboBox* rendererCombo = dynamic_cast<QComboBox*>(table->cellWidget(row, 1));
        int rendererIdx = rendererCombo->findData(renderers[type].toString());
        rendererCombo->setCurrentIndex(rendererIdx >= 0 ? rendererIdx : 0);

        row++;
    }
}

QVariant CellRendererTableToHash::getWidgetConfigValue(QWidget* widget, bool& ok)
{
    QHash<QString, QString> config;
    CellRendererTable* table = dynamic_cast<CellRendererTable*>(widget);
    for (int row = 0, total = table->rowCount(); row < total; row++)
    {
        QTableWidgetItem* typeItem = table->item(row, 0);
        QComboBox* rendererCombo = dynamic_cast<QComboBox*>(table->cellWidget(row, 1));
        if (!typeItem || !rendererCombo)
        {
            ok = false;
            return QVariant();
        }
        config[typeItem->text()] = rendererCombo->currentData().toString();
    }

    ok = true;
    return QVariant::fromValue(config);
}

const char* CellRendererTableToHash::getModifiedNotifier() const
{
    return SIGNAL(modified());
}

QString CellRendererTableToHash::getFilterString(QWidget* widget) const
{
    return ConfigDialog::getFilterString(widget);
}
