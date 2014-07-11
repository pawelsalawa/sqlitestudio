#ifndef UIPLUGINLOADINGHANDLERIMPL_H
#define UIPLUGINLOADINGHANDLERIMPL_H

#include "services/impl/pluginmanagerimpl.h"

class UiPluginLoadingHandlerImpl : public PluginLoadingHandler
{
    public:
        QPluginLoader* createLoader(const QString& fileName) const;
};

#endif // UIPLUGINLOADINGHANDLERIMPL_H
