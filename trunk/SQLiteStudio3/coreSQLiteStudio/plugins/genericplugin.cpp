#include "genericplugin.h"
#include "services/pluginmanager.h"
#include <QMetaClassInfo>

QString GenericPlugin::getName() const
{
    return metaData["name"].toString();
}

QString GenericPlugin::getTitle() const
{
    if (!metaData["title"].isValid())
        return getName();

    return metaData["title"].toString();
}

CfgMain* GenericPlugin::getMainUiConfig()
{
    return nullptr;
}

QString GenericPlugin::getDescription() const
{
    return metaData["description"].toString();
}

int GenericPlugin::getVersion() const
{
    return metaData["version"].toInt();
}

QString GenericPlugin::getPrintableVersion() const
{
    return PLUGINS->toPrintableVersion(getVersion());
}

bool GenericPlugin::init()
{
    return true;
}

void GenericPlugin::deinit()
{
}

void GenericPlugin::loadMetaData(const QJsonObject& metaData)
{
    this->metaData = PLUGINS->readMetaData(metaData);
}

const char* GenericPlugin::getMetaInfo(const QString& key) const
{
    for (int i = 0; i < metaObject()->classInfoCount(); i++)
    {
        if (key != metaObject()->classInfo(i).name())
            continue;

        return metaObject()->classInfo(i).value();
    }
    return nullptr;
}

QString GenericPlugin::getAuthor() const
{
    return metaData["author"].toString();
}
