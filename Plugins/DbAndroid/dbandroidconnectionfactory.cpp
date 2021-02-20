#include "dbandroidconnectionfactory.h"
#include "dbandroidjsonconnection.h"
#include "dbandroidshellconnection.h"

DbAndroidConnectionFactory::DbAndroidConnectionFactory(DbAndroid* plugin) :
    plugin(plugin)
{
}

DbAndroidConnection* DbAndroidConnectionFactory::create(const QString& url, QObject* parent)
{
    return create(DbAndroidUrl(url), parent);
}

DbAndroidConnection* DbAndroidConnectionFactory::create(const DbAndroidUrl& url, QObject* parent)
{
    switch (url.getMode())
    {
        case DbAndroidMode::SHELL:
            return new DbAndroidShellConnection(plugin, url.getDevice(), parent);
        case DbAndroidMode::NETWORK:
        case DbAndroidMode::USB:
            return new DbAndroidJsonConnection(plugin, parent);
        case DbAndroidMode::null:
            break;
    }
    return nullptr;
}
