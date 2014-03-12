#ifndef PLUGINTYPE_H
#define PLUGINTYPE_H

#include "coreSQLiteStudio_global.h"
#include <QList>
#include <QString>
#include <QObject>

class Plugin;

template <class T>
class DefinedPluginType;

class API_EXPORT PluginType
{
    public:
        virtual ~PluginType();

        QString getName() const;
        QString getTitle() const;
        QString getConfigUiForm() const;
        QList<Plugin*> getLoadedPlugins() const;
        QStringList getAllPluginNames() const;

        virtual bool test(Plugin* plugin) = 0;

        template <class T>
        bool isForPluginType()
        {
            return dynamic_cast<const DefinedPluginType<T>*>(this) != nullptr;
        }

    protected:
        PluginType(const QString& title, const QString& form);

        virtual QString typeName() const = 0;

        QString title;
        QString configUiForm;
};

template <class T>
class DefinedPluginType : public PluginType
{
    friend class PluginManager;

    public:
        bool test(Plugin* plugin)
        {
            return (dynamic_cast<T*>(plugin) != nullptr);
        }

        QString typeName() const
        {
            return typeid(T).name();
        }

    protected:
        DefinedPluginType(const QString& title, const QString& form) : PluginType(title, form) {}
};

#endif // PLUGINTYPE_H
