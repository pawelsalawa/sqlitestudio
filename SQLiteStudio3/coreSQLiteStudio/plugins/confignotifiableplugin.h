#ifndef CONFIGNOTIFIABLEPLUGIN_H
#define CONFIGNOTIFIABLEPLUGIN_H

#include <QVariant>

class CfgEntry;

class ConfigNotifiablePlugin
{
    public:
        virtual void configModified(CfgEntry* key, const QVariant& value) = 0;
};

#endif // CONFIGNOTIFIABLEPLUGIN_H
