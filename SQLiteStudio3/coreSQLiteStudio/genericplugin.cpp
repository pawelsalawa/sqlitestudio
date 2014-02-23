#include "genericplugin.h"
#include <QMetaClassInfo>

QString GenericPlugin::getName() const
{
    return metaObject()->className();
}

QString GenericPlugin::getTitle() const
{
    const char *title = getMetaInfo("title");
    if (!title)
        return getName();

    return title;
}

QString GenericPlugin::getConfigUiForm() const
{
    return getMetaInfo("ui");
}

QString GenericPlugin::getDescription() const
{
    return getMetaInfo("description");
}

int GenericPlugin::getVersion() const
{
    return QString(getMetaInfo("version")).toInt();
}

QString GenericPlugin::getPrintableVersion() const
{
    static const QString versionStr = "%1.%2.%3";
    int version = getVersion();
    return versionStr.arg(version / 10000)
                     .arg(version / 100 % 100)
                     .arg(version % 100);
}

bool GenericPlugin::init()
{
    return true;
}

void GenericPlugin::deinit()
{
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
    return getMetaInfo("author");
}
