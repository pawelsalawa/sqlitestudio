#include "configmapper.h"
#include "config_builder.h"
#include "services/config.h"
#include "services/pluginmanager.h"
#include "customconfigwidgetplugin.h"
#include "common/colorbutton.h"
#include "common/fontedit.h"
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QPlainTextEdit>
#include <QListWidget>
#include <QTableWidget>
#include <QCheckBox>
#include <QGroupBox>
#include <QDebug>
#include <QStringListModel>
#include <QFontComboBox>
#include <QKeySequenceEdit>
#include <common/configcombobox.h>
#include <common/configradiobutton.h>
#include <common/fileedit.h>

#define APPLY_CFG(Widget, Value, WidgetType, Method, DataType) \
    APPLY_CFG_VARIANT(Widget, Value.value<DataType>(), WidgetType, Method)

#define APPLY_CFG_COND(Widget, Value, WidgetType, Method, DataType, ExtraConditionMethod) \
    if (qobject_cast<WidgetType*>(Widget) && qobject_cast<WidgetType*>(Widget)->ExtraConditionMethod())\
    {\
        qobject_cast<WidgetType*>(Widget)->Method(Value.value<DataType>());\
        return;\
    }

#define APPLY_CFG_VARIANT(Widget, Value, WidgetType, Method) \
    if (qobject_cast<WidgetType*>(Widget))\
    {\
        qobject_cast<WidgetType*>(Widget)->Method(Value);\
        return;\
    }

#define APPLY_NOTIFIER(Widget, Key, WidgetType, Notifier) \
    if (qobject_cast<WidgetType*>(Widget))\
    {\
        connect(Widget, Notifier, this, SLOT(handleModified()));\
        if (Widget->property(CFG_NOTIFY_PROPERTY).isValid() && Widget->property(CFG_NOTIFY_PROPERTY).toBool())\
            connect(Widget, Notifier, this, SLOT(uiConfigEntryChanged()));\
        \
        if (Widget->property(CFG_PREVIEW_PROPERTY).isValid() && Widget->property(CFG_PREVIEW_PROPERTY).toBool())\
            connect(Key, SIGNAL(changed(QVariant)), this, SLOT(notifiableConfigKeyChanged()));\
        \
        return;\
    }

#define APPLY_NOTIFIER_COND(Widget, Key, WidgetType, Notifier, ExtraConditionMethod) \
    if (qobject_cast<WidgetType*>(Widget) && qobject_cast<WidgetType*>(Widget)->ExtraConditionMethod())\
    {\
        connect(Widget, Notifier, this, SLOT(handleModified()));\
        if (Widget->property(CFG_NOTIFY_PROPERTY).isValid() && Widget->property(CFG_NOTIFY_PROPERTY).toBool())\
            connect(Widget, Notifier, this, SLOT(uiConfigEntryChanged()));\
        \
        if (Widget->property(CFG_PREVIEW_PROPERTY).isValid() && Widget->property(CFG_PREVIEW_PROPERTY).toBool())\
            connect(Key, SIGNAL(changed(QVariant)), this, SLOT(notifiableConfigKeyChanged()));\
        \
        return;\
    }

#define GET_CFG_VALUE(Widget, Key, WidgetType, Method) \
    if (qobject_cast<WidgetType*>(Widget))\
        return qobject_cast<WidgetType*>(Widget)->Method();

#define GET_CFG_VALUE_COND(Widget, Key, WidgetType, Method, ExtraConditionMethod) \
    if (qobject_cast<WidgetType*>(Widget) && qobject_cast<WidgetType*>(Widget)->ExtraConditionMethod())\
        return qobject_cast<WidgetType*>(Widget)->Method();

ConfigMapper::ConfigMapper(CfgMain* cfgMain)
{
    this->cfgMainList << cfgMain;
}

ConfigMapper::ConfigMapper(const QList<CfgMain*> cfgMain) :
    cfgMainList(cfgMain)
{
}


void ConfigMapper::applyCommonConfigToWidget(QWidget *widget, const QVariant &value, CfgEntry* cfgEntry)
{
    APPLY_CFG(widget, value, QCheckBox, setChecked, bool);
    APPLY_CFG(widget, value, QLineEdit, setText, QString);
    APPLY_CFG(widget, value, QTextEdit, setPlainText, QString);
    APPLY_CFG(widget, value, QPlainTextEdit, setPlainText, QString);
    APPLY_CFG(widget, value, QSpinBox, setValue, int);
    APPLY_CFG(widget, value, QFontComboBox, setCurrentFont, QFont);
    APPLY_CFG(widget, value, FontEdit, setFont, QFont);
    APPLY_CFG(widget, value, ColorButton, setColor, QColor);
    APPLY_CFG(widget, value, FileEdit, setFile, QString);
    APPLY_CFG_VARIANT(widget, QKeySequence::fromString(value.toString()), QKeySequenceEdit, setKeySequence);
    APPLY_CFG_VARIANT(widget, value, ConfigRadioButton, alignToValue);
    APPLY_CFG_COND(widget, value, QGroupBox, setChecked, bool, isCheckable);

    // ComboBox needs special treatment, cause setting its value might not be successful (value not in valid values)
    QComboBox* cb = dynamic_cast<QComboBox*>(widget);
    if (cb)
    {
        if (cfgEntry->get().type() == QVariant::Int)
        {
            cb->setCurrentIndex(value.toInt());
            if (cb->currentIndex() != value.toInt())
                cfgEntry->set(cb->currentIndex());
        }
        else
        {
            cb->setCurrentText(value.toString());
            if (cb->currentText() != value.toString())
                cfgEntry->set(cb->currentText());
        }
        return;
    }

    qWarning() << "Unhandled config widget type (for APPLY_CFG):" << widget->metaObject()->className()
               << "with value:" << value;
}

void ConfigMapper::connectCommonNotifierToWidget(QWidget* widget, CfgEntry* key)
{
    APPLY_NOTIFIER(widget, key, QCheckBox, SIGNAL(stateChanged(int)));
    APPLY_NOTIFIER(widget, key, QLineEdit, SIGNAL(textChanged(QString)));
    APPLY_NOTIFIER(widget, key, QTextEdit, SIGNAL(textChanged()));
    APPLY_NOTIFIER(widget, key, QPlainTextEdit, SIGNAL(textChanged()));
    APPLY_NOTIFIER(widget, key, QSpinBox, SIGNAL(valueChanged(QString)));
    APPLY_NOTIFIER(widget, key, QFontComboBox, SIGNAL(currentFontChanged(QFont)));
    APPLY_NOTIFIER(widget, key, FontEdit, SIGNAL(fontChanged(QFont)));
    APPLY_NOTIFIER(widget, key, FileEdit, SIGNAL(fileChanged(QString)));
    APPLY_NOTIFIER(widget, key, QKeySequenceEdit, SIGNAL(editingFinished()));
    APPLY_NOTIFIER(widget, key, ColorButton, SIGNAL(colorChanged(QColor)));
    APPLY_NOTIFIER(widget, key, ConfigRadioButton, SIGNAL(toggledOn(QVariant)));
    APPLY_NOTIFIER_COND(widget, key, QGroupBox, SIGNAL(clicked(bool)), isCheckable);
    if (key->get().type() == QVariant::Int)
    {
        APPLY_NOTIFIER(widget, key, QComboBox, SIGNAL(currentIndexChanged(int)));
    }
    else
    {
        APPLY_NOTIFIER(widget, key, QComboBox, SIGNAL(currentTextChanged(QString)));
    }

    qWarning() << "Unhandled config widget type (for APPLY_NOTIFIER):" << widget->metaObject()->className();
}

void ConfigMapper::saveCommonConfigFromWidget(QWidget* widget, CfgEntry* key)
{
    bool ok = false;
    QVariant value = getCommonConfigValueFromWidget(widget, key, ok);
    if (ok)
        key->set(value);
}

QVariant ConfigMapper::getCommonConfigValueFromWidget(QWidget* widget, CfgEntry* key, bool& ok)
{
    ok = true;
    GET_CFG_VALUE(widget, key, QCheckBox, isChecked);
    GET_CFG_VALUE(widget, key, QLineEdit, text);
    GET_CFG_VALUE(widget, key, QTextEdit, toPlainText);
    GET_CFG_VALUE(widget, key, QPlainTextEdit, toPlainText);
    GET_CFG_VALUE(widget, key, QSpinBox, value);
    GET_CFG_VALUE(widget, key, QFontComboBox, currentFont);
    GET_CFG_VALUE(widget, key, FontEdit, getFont);
    GET_CFG_VALUE(widget, key, FileEdit, getFile);
    GET_CFG_VALUE(widget, key, QKeySequenceEdit, keySequence().toString);
    GET_CFG_VALUE(widget, key, ColorButton, getColor);
    GET_CFG_VALUE_COND(widget, key, ConfigRadioButton, getAssignedValue, isChecked);
    GET_CFG_VALUE_COND(widget, key, QGroupBox, isChecked, isCheckable);
    if (key->get().type() == QVariant::Int)
    {
        GET_CFG_VALUE(widget, key, QComboBox, currentIndex);
    }
    else
    {
        GET_CFG_VALUE(widget, key, QComboBox, currentText);
    }

    qWarning() << "Unhandled config widget type (for GET_CFG_VALUE):" << widget->metaObject()->className();
    ok = false;
    return QVariant();
}

QVariant ConfigMapper::getCustomConfigValueFromWidget(QWidget* widget, CfgEntry* key, bool& ok)
{
    CustomConfigWidgetPlugin* plugin;
    QList<CustomConfigWidgetPlugin*> handlers;
    handlers += internalCustomConfigWidgets;
    handlers += PLUGINS->getLoadedPlugins<CustomConfigWidgetPlugin>();

    foreach (plugin, handlers)
    {
        if (plugin->isConfigForWidget(key, widget))
            return plugin->getWidgetConfigValue(widget, ok);
    }

    ok = false;
    return QVariant();
}

QVariant ConfigMapper::getConfigValueFromWidget(QWidget* widget, CfgEntry* key)
{
    bool ok;
    QVariant value = getCustomConfigValueFromWidget(widget, key, ok);
    if (!ok)
        value = getCommonConfigValueFromWidget(widget, key, ok);

    return value;
}

void ConfigMapper::loadToWidget(QWidget *topLevelWidget)
{
    QHash<QString, CfgEntry *> allConfigEntries = getAllConfigEntries();
    QList<QWidget*> allConfigWidgets = getAllConfigWidgets(topLevelWidget) + extraWidgets;
    QHash<QString,QVariant> config;

    if (isPersistant())
        config = CFG->getAll();

    updatingEntry = true;
    foreach (QWidget* widget, allConfigWidgets)
        applyConfigToWidget(widget, allConfigEntries, config);

    updatingEntry = false;
}

void ConfigMapper::loadToWidget(CfgEntry* config, QWidget* widget)
{
    QVariant configValue = config->get();

    updatingEntry = true;
    if (applyCustomConfigToWidget(config, widget, configValue))
        return;

    applyCommonConfigToWidget(widget, configValue, config);
    updatingEntry = false;
}

void ConfigMapper::saveFromWidget(QWidget *widget, bool noTransaction)
{
    QHash<QString, CfgEntry *> allConfigEntries = getAllConfigEntries();
    QList<QWidget*> allConfigWidgets = getAllConfigWidgets(widget);

    if (!noTransaction && isPersistant())
        CFG->beginMassSave();

    foreach (QWidget* w, allConfigWidgets)
        saveWidget(w, allConfigEntries);

    if (!noTransaction && isPersistant())
        CFG->commitMassSave();
}


void ConfigMapper::applyConfigToWidget(QWidget* widget, const QHash<QString, CfgEntry *> &allConfigEntries, const QHash<QString,QVariant>& config)
{
    CfgEntry* cfgEntry = getConfigEntry(widget, allConfigEntries);
    if (!cfgEntry)
        return;

    QVariant configValue;
    if (config.contains(cfgEntry->getFullKey()))
    {
        configValue = config[cfgEntry->getFullKey()];
        if (!configValue.isValid())
            configValue = cfgEntry->getDefultValue();
    }
    else if (cfgEntry->isPersistable())
    {
        // In case this is a persistable config, we should have everything in the config hash, which is just one, initial database query.
        // If we don't, than we don't want to call get(), because it will cause one more query to the database for it.
        // We go with the default value.
        configValue = cfgEntry->getDefultValue();
    }
    else
    {
        // Non persistable entries will return whatever they have, without querying database.
        configValue = cfgEntry->get();
    }

    widgetToConfigEntry.insert(widget, cfgEntry);
    configEntryToWidgets.insertMulti(cfgEntry, widget);

    handleSpecialWidgets(widget, allConfigEntries);

    if (!connectCustomNotifierToWidget(widget, cfgEntry))
        connectCommonNotifierToWidget(widget, cfgEntry);

    applyConfigToWidget(widget, cfgEntry, configValue);
}

void ConfigMapper::applyConfigToWidget(QWidget* widget, CfgEntry* cfgEntry, const QVariant& configValue)
{
    if (applyCustomConfigToWidget(cfgEntry, widget, configValue))
        return;

    applyCommonConfigToWidget(widget, configValue, cfgEntry);
}

void ConfigMapper::handleSpecialWidgets(QWidget* widget, const QHash<QString, CfgEntry*>& allConfigEntries)
{
    handleConfigComboBox(widget, allConfigEntries);
}

void ConfigMapper::handleConfigComboBox(QWidget* widget, const QHash<QString, CfgEntry*>& allConfigEntries)
{
    ConfigComboBox* ccb = dynamic_cast<ConfigComboBox*>(widget);
    if (!ccb)
        return;

    CfgEntry* key = getEntryForProperty(widget, "modelName", allConfigEntries);
    if (!key)
        return;

    QStringList list = key->get().toStringList();
    ccb->setModel(new QStringListModel(list));

    if (realTimeUpdates)
    {
        specialConfigEntryToWidgets.insertMulti(key, widget);
        connect(key, SIGNAL(changed(QVariant)), this, SLOT(updateConfigComboModel(QVariant)));
    }
}

bool ConfigMapper::applyCustomConfigToWidget(CfgEntry* key, QWidget* widget, const QVariant& value)
{
    CustomConfigWidgetPlugin* handler;
    QList<CustomConfigWidgetPlugin*> handlers;
    handlers += internalCustomConfigWidgets;
    handlers += PLUGINS->getLoadedPlugins<CustomConfigWidgetPlugin>();

    foreach (handler, handlers)
    {
        if (handler->isConfigForWidget(key, widget))
        {
            handler->applyConfigToWidget(key, widget, value);
            return true;
        }
    }
    return false;
}

bool ConfigMapper::connectCustomNotifierToWidget(QWidget* widget, CfgEntry* cfgEntry)
{
    CustomConfigWidgetPlugin* handler;
    QList<CustomConfigWidgetPlugin*> handlers;
    handlers += internalCustomConfigWidgets;
    handlers += PLUGINS->getLoadedPlugins<CustomConfigWidgetPlugin>();

    foreach (handler, handlers)
    {
        if (handler->isConfigForWidget(cfgEntry, widget))
        {
            connect(widget, handler->getModifiedNotifier(), this, SIGNAL(modified()));
            if (widget->property(CFG_NOTIFY_PROPERTY).isValid() && widget->property(CFG_NOTIFY_PROPERTY).toBool())
                connect(widget, handler->getModifiedNotifier(), this, SLOT(uiConfigEntryChanged()));

            if (widget->property(CFG_PREVIEW_PROPERTY).isValid() && widget->property(CFG_PREVIEW_PROPERTY).toBool())
                connect(cfgEntry, SIGNAL(changed(QVariant)), this, SLOT(notifiableConfigKeyChanged()));

            return true;
        }
    }
    return false;
}

void ConfigMapper::saveWidget(QWidget* widget, const QHash<QString, CfgEntry *> &allConfigEntries)
{
    CfgEntry* cfgEntry = getConfigEntry(widget, allConfigEntries);
    if (!cfgEntry)
        return;

    saveFromWidget(widget, cfgEntry);
}

void ConfigMapper::saveFromWidget(QWidget* widget, CfgEntry* cfgEntry)
{
    if (saveCustomConfigFromWidget(widget, cfgEntry))
        return;

    saveCommonConfigFromWidget(widget, cfgEntry);
}


bool ConfigMapper::saveCustomConfigFromWidget(QWidget* widget, CfgEntry* key)
{
    CustomConfigWidgetPlugin* plugin;
    QList<CustomConfigWidgetPlugin*> handlers;
    handlers += internalCustomConfigWidgets;
    handlers += PLUGINS->getLoadedPlugins<CustomConfigWidgetPlugin>();

    foreach (plugin, handlers)
    {
        if (plugin->isConfigForWidget(key, widget))
        {
            bool ok = false;
            QVariant value = plugin->getWidgetConfigValue(widget, ok);
            if (!ok)
                return false;

            key->set(value);
            return true;
        }
    }
    return false;
}

CfgEntry* ConfigMapper::getConfigEntry(QWidget* widget, const QHash<QString, CfgEntry*>& allConfigEntries)
{
    return getEntryForProperty(widget, CFG_MODEL_PROPERTY, allConfigEntries);
}

CfgEntry* ConfigMapper::getEntryForProperty(QWidget* widget, const char* propertyName, const QHash<QString, CfgEntry*>& allConfigEntries)
{
    QString key = widget->property(propertyName).toString();
    if (!allConfigEntries.contains(key))
    {
        qCritical() << "Config entries don't contain key" << key
                    << "but it was requested by ConfigMapper::getEntryForProperty() for widget"
                    << widget->metaObject()->className() << "::" << widget->objectName();
        return nullptr;
    }

    return allConfigEntries[key];
}

QHash<QString, CfgEntry *> ConfigMapper::getAllConfigEntries()
{
    if (allEntries.isEmpty())
    {
        QString key;
        for (CfgMain* cfgMain : cfgMainList)
        {
            QHashIterator<QString,CfgCategory*> catIt(cfgMain->getCategories());
            while (catIt.hasNext())
            {
                catIt.next();
                QHashIterator<QString,CfgEntry*> entryIt( catIt.value()->getEntries());
                while (entryIt.hasNext())
                {
                    entryIt.next();
                    key = catIt.key()+"."+entryIt.key();
                    if (allEntries.contains(key))
                    {
                        qCritical() << "Duplicate config entry key:" << key;
                        continue;
                    }
                    allEntries[key] = entryIt.value();
                }
            }
        }
    }

    return allEntries;
}

QList<QWidget*> ConfigMapper::getAllConfigWidgets(QWidget *parent)
{
    QList<QWidget*> results;
    QWidget* widget = nullptr;
    foreach (QObject* obj, parent->children())
    {
        widget = qobject_cast<QWidget*>(obj);
        if (!widget)
            continue;

        results += getAllConfigWidgets(widget);
        if (!widget->property(CFG_MODEL_PROPERTY).isValid())
            continue;

        results << widget;
    }
    return results;
}

bool ConfigMapper::isPersistant() const
{
    for (CfgMain* cfgMain : cfgMainList)
    {
        if (cfgMain->isPersistable())
            return true;
    }
    return false;
}
QList<QWidget *> ConfigMapper::getExtraWidgets() const
{
    return extraWidgets;
}

void ConfigMapper::setExtraWidgets(const QList<QWidget *> &value)
{
    extraWidgets = value;
}

void ConfigMapper::addExtraWidget(QWidget *w)
{
    extraWidgets << w;
}

void ConfigMapper::addExtraWidgets(const QList<QWidget *> &list)
{
    extraWidgets += list;
}

void ConfigMapper::clearExtraWidgets()
{
    extraWidgets.clear();
}

void ConfigMapper::handleModified()
{
    if (realTimeUpdates && !updatingEntry)
    {
        QWidget* widget = dynamic_cast<QWidget*>(sender());
        if (widget && widgetToConfigEntry.contains(widget))
        {
            updatingEntry = true;
            saveFromWidget(widget, widgetToConfigEntry.value(widget));
            updatingEntry = false;
        }
    }
    emit modified();
}

void ConfigMapper::entryChanged(const QVariant& newValue)
{
    // This is called only when bindToConfig() was used.
    if (updatingEntry)
        return;

    CfgEntry* cfgEntry = dynamic_cast<CfgEntry*>(sender());
    if (!cfgEntry)
    {
        qCritical() << "entryChanged() invoked by object that is not CfgEntry:" << sender();
        return;
    }

    if (!configEntryToWidgets.contains(cfgEntry))
        return;

    updatingEntry = true;
    for (QWidget* w : configEntryToWidgets.values(cfgEntry))
        applyConfigToWidget(w, cfgEntry, newValue);

    updatingEntry = false;
}

void ConfigMapper::uiConfigEntryChanged()
{
    if (updatingEntry)
        return;

    QWidget* w = dynamic_cast<QWidget*>(sender());
    if (!w)
    {
        qWarning() << "ConfigMapper::uiConfigEntryChanged() called not from widget:" << sender();
        return;
    }

    if (!widgetToConfigEntry.contains(w))
    {
        qWarning() << "ConfigMapper::uiConfigEntryChanged() called with widget that has no key assigned:" << w;
        return;
    }

    CfgEntry* key = widgetToConfigEntry[w];
    QVariant value = getConfigValueFromWidget(w, key);
    emit notifyEnabledWidgetModified(w, key, value);
}

void ConfigMapper::updateConfigComboModel(const QVariant& value)
{
    CfgEntry* key = dynamic_cast<CfgEntry*>(sender());
    if (!specialConfigEntryToWidgets.contains(key))
        return;

    QWidget* w = specialConfigEntryToWidgets[key];
    ConfigComboBox* ccb = dynamic_cast<ConfigComboBox*>(w);
    if (!w)
        return;

    QString cText = ccb->currentText();
    QStringList newList = value.toStringList();
    ccb->setModel(new QStringListModel(newList));
    if (newList.contains(cText))
        ccb->setCurrentText(cText);
}

void ConfigMapper::notifiableConfigKeyChanged()
{
    CfgEntry* key = dynamic_cast<CfgEntry*>(sender());
    if (!key)
    {
        qCritical() << "ConfigMapper::notifiableConfigKeyChanged() called not from CfgEntry";
        return;
    }

    if (!configEntryToWidgets.contains(key))
    {
        qCritical() << "No entry in configEntryToWidgets for key:" << key->getFullKey();
        return;
    }

    loadToWidget(key, configEntryToWidgets[key]);
}

void ConfigMapper::bindToConfig(QWidget* topLevelWidget)
{
    realTimeUpdates = true;
    loadToWidget(topLevelWidget);
    for (CfgEntry* cfgEntry : configEntryToWidgets.keys())
        connect(cfgEntry, SIGNAL(changed(QVariant)), this, SLOT(entryChanged(QVariant)));
}

void ConfigMapper::unbindFromConfig()
{
    for (CfgEntry* cfgEntry : configEntryToWidgets.keys())
        disconnect(cfgEntry, SIGNAL(changed(QVariant)), this, SLOT(entryChanged(QVariant)));

    for (CfgEntry* cfgEntry : specialConfigEntryToWidgets.keys())
        disconnect(cfgEntry, SIGNAL(changed(QVariant)), this, SLOT(entryChanged(QVariant)));

    configEntryToWidgets.clear();
    widgetToConfigEntry.clear();
    specialConfigEntryToWidgets.clear();
    realTimeUpdates = false;
}

QWidget* ConfigMapper::getBindWidgetForConfig(CfgEntry* key) const
{
    if (configEntryToWidgets.contains(key))
        return configEntryToWidgets[key];

    return nullptr;
}

CfgEntry* ConfigMapper::getBindConfigForWidget(QWidget* widget) const
{
    if (widgetToConfigEntry.contains(widget))
        return widgetToConfigEntry.value(widget);

    return nullptr;
}

void ConfigMapper::setInternalCustomConfigWidgets(const QList<CustomConfigWidgetPlugin*>& value)
{
    internalCustomConfigWidgets = value;
}

