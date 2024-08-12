#include "collationmanagerimpl.h"
#include "services/pluginmanager.h"
#include "plugins/scriptingplugin.h"
#include "services/notifymanager.h"
#include "services/dbmanager.h"
#include "common/utils.h"
#include <QDebug>

class CollationFunctionInfoImpl : public ScriptingPlugin::FunctionInfo
{
    public:
        QString getName() const {return QString();}
        QStringList getArguments() const {return {"first", "second"};}
        bool getUndefinedArgs() const {return false;}
};

CollationFunctionInfoImpl collationFunctionInfo;

CollationManagerImpl::CollationManagerImpl()
{
    init();
}

void CollationManagerImpl::setCollations(const QList<CollationManager::CollationPtr>& newCollations)
{
    collations = newCollations;
    refreshCollationsByKey();
    storeInConfig();
    emit collationListChanged();
}

QList<CollationManager::CollationPtr> CollationManagerImpl::getAllCollations() const
{
    return collations;
}

CollationManager::CollationPtr CollationManagerImpl::getCollation(const QString &name) const
{
   if (!collationsByKey.contains(name))
   {
       qCritical() << "Could not find requested collation" << name << ".";
       return nullptr;
   }
   return collationsByKey[name];
}

QList<CollationManager::CollationPtr> CollationManagerImpl::getCollationsForDatabase(const QString& dbName) const
{
    QList<CollationPtr> results;
    for (const CollationPtr& coll : collations)
    {
        if (coll->allDatabases || coll->databases.contains(dbName, Qt::CaseInsensitive))
            results << coll;
    }
    return results;
}

int CollationManagerImpl::evaluate(const QString& name, const QString& value1, const QString& value2)
{
    if (!collationsByKey.contains(name))
    {
        qWarning() << "Could not find requested collation" << name << ", so using default collation.";
        return evaluateDefault(value1, value2);
    }

    ScriptingPlugin* plugin = PLUGINS->getScriptingPlugin(collationsByKey[name]->lang);
    if (!plugin)
    {
        qWarning() << "Plugin for collation" << name << ", not loaded, so using default collation.";
        return evaluateDefault(value1, value2);
    }

    QString err;
    QVariant result = plugin->evaluate(collationsByKey[name]->code, collationFunctionInfo, {value1, value2}, &err);

    if (!err.isNull())
    {
        qWarning() << "Error while evaluating collation:" << err;
        return evaluateDefault(value1, value2);
    }

    bool ok;
    int intResult = result.toInt(&ok);
    if (!ok)
    {
        qWarning() << "Not integer result from collation:" << result.toString();
        return evaluateDefault(value1, value2);
    }

    return intResult;
}

int CollationManagerImpl::evaluateDefault(const QString& value1, const QString& value2)
{
    return value1.compare(value2, Qt::CaseInsensitive);
}

void CollationManagerImpl::init()
{
    loadFromConfig();
    refreshCollationsByKey();
}

void CollationManagerImpl::storeInConfig()
{
    QVariantList list;
    QHash<QString,QVariant> collHash;
    for (CollationPtr coll : collations)
    {
        collHash["name"] = coll->name;
        collHash["type"] = coll->type;
        collHash["lang"] = coll->lang;
        collHash["code"] = coll->code;
        collHash["allDatabases"] = coll->allDatabases;
        collHash["databases"] =common(DBLIST->getDbNames(),  coll->databases);
        list << collHash;
    }
    CFG_CORE.Internal.Collations.set(list);
}

void CollationManagerImpl::loadFromConfig()
{
    collations.clear();

    QVariantList list = CFG_CORE.Internal.Collations.get();
    QHash<QString,QVariant> collHash;
    CollationPtr coll;
    for (const QVariant& var : list)
    {
        collHash = var.toHash();
        coll = CollationPtr::create();
        coll->name = collHash["name"].toString();
        if (collHash.contains("type") && collHash["type"].toInt() == CollationType::EXTENSION_BASED)
            coll->type = CollationType::EXTENSION_BASED;
        else
            coll->type = CollationType::FUNCTION_BASED;
        coll->lang = updateScriptingQtLang(collHash["lang"].toString());
        coll->code = collHash["code"].toString();
        coll->databases = collHash["databases"].toStringList();
        coll->allDatabases = collHash["allDatabases"].toBool();
        collations << coll;
    }
}

void CollationManagerImpl::refreshCollationsByKey()
{
    collationsByKey.clear();
    for (CollationPtr collation : collations)
        collationsByKey[collation->name] = collation;
}

QString CollationManagerImpl::updateScriptingQtLang(const QString& lang) const
{
    if (lang == "QtScript")
        return QStringLiteral("JavaScript");

    return lang;
}
