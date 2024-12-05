#ifndef COLLATIONMANAGERIMPL_H
#define COLLATIONMANAGERIMPL_H

#include <QHash>

#include "services/collationmanager.h"

class ScriptingPlugin;
class Plugin;
class PluginType;

class API_EXPORT CollationManagerImpl : public CollationManager
{
    public:
        CollationManagerImpl();

        void setCollations(const QList<CollationPtr>& newCollations);
        QList<CollationPtr> getAllCollations() const;
        QList<CollationPtr> getCollationsForDatabase(const QString& dbName) const;
        int evaluate(const QString& name, const QString& value1, const QString& value2);
        int evaluateDefault(const QString& value1, const QString& value2);
        CollationPtr getCollation(const QString &name) const;

    private:
        void init();
        void storeInConfig();
        void loadFromConfig();
        void refreshCollationsByKey();
        QString updateScriptingQtLang(const QString& lang) const;

        QList<CollationPtr> collations;
        QHash<QString,CollationPtr> collationsByKey;
        QHash<QString,ScriptingPlugin*> scriptingPlugins;

    private slots:
        void pluginLoaded(Plugin* plugin, PluginType* type);
        void pluginUnloaded(Plugin* plugin, PluginType* type);
};

#endif // COLLATIONMANAGERIMPL_H
