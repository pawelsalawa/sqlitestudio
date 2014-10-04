#include "builtinplugin.h"
#include "services/pluginmanager.h"
#include <QMetaClassInfo>

QString BuiltInPlugin::getName() const
{
    return metaObject()->className();
}

QString BuiltInPlugin::getTitle() const
{
    const char *title = getMetaInfo("title");
    if (!title)
        return getName();

    return title;
}

QString BuiltInPlugin::getDescription() const
{
    return getMetaInfo("description");
}

int BuiltInPlugin::getVersion() const
{
    return QString(getMetaInfo("version")).toInt();
}

QString BuiltInPlugin::getPrintableVersion() const
{
    return PLUGINS->toPrintableVersion(getVersion());
}

QString BuiltInPlugin::getAuthor() const
{
    return getMetaInfo("author");
}

bool BuiltInPlugin::init()
{
    return true;
}

void BuiltInPlugin::deinit()
{
}

const char* BuiltInPlugin::getMetaInfo(const QString& key) const
{
    for (int i = 0; i < metaObject()->classInfoCount(); i++)
    {
        if (key != metaObject()->classInfo(i).name())
            continue;

        return metaObject()->classInfo(i).value();
    }
    return nullptr;
}
