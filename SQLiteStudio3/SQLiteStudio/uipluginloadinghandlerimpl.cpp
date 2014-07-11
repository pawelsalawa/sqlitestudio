#include "uipluginloadinghandlerimpl.h"
#include <QPluginLoader>

QPluginLoader* UiPluginLoadingHandlerImpl::createLoader(const QString& fileName) const
{
    return new QPluginLoader(fileName);
}
