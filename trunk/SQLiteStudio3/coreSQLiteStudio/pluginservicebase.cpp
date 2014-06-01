#include "pluginservicebase.h"

PluginServiceBase::PluginServiceBase(QObject *parent) :
    QObject(parent)
{
}

void PluginServiceBase::handleValidationFromPlugin(bool configValid, CfgEntry* key, const QString& errorMessage)
{
    emit validationResultFromPlugin(configValid, key, errorMessage);
}

void PluginServiceBase::updateVisibilityAndEnabled(CfgEntry* key, bool visible, bool enabled)
{
    emit stateUpdateRequestFromPlugin(key, visible, enabled);
}
