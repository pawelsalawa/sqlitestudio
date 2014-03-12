#include "cfginternals.h"
#include "services/config.h"
#include <QDebug>

CfgCategory* lastCreatedCfgCategory = nullptr;
CfgMain* lastCreatedCfgMain = nullptr;
QList<CfgMain*> CfgMain::instances;

CfgMain::CfgMain(const QString& name) :
    name(name)
{
    lastCreatedCfgMain = this;
    instances << this;
}

void CfgMain::staticInit()
{
    qRegisterMetaType<CfgMain*>("CfgMain*");
    qRegisterMetaType<CfgCategory*>("CfgCategory*");
    qRegisterMetaType<CfgEntry*>("CfgEntry*");
}

QList<CfgMain*> CfgMain::getInstances()
{
    return instances;
}

QHash<QString, CfgCategory *> &CfgMain::getCategories()
{
    return childs;
}

CfgCategory::CfgCategory(const QString &name)
{
    this->name = name;
    lastCreatedCfgCategory = this;
    lastCreatedCfgMain->childs[name] = this;
}

QString CfgCategory::toString() const
{
    return name;
}

QHash<QString, CfgEntry *> &CfgCategory::getEntries()
{
    return childs;
}

CfgCategory::operator QString() const
{
    return name;
}

CfgEntry::CfgEntry(const CfgEntry& other) :
    QObject(), parent(other.parent), name(other.name), dbKey(other.dbKey), defValue(other.defValue),
    defValueFunc(other.defValueFunc)
{
}

CfgEntry::CfgEntry(const QString &name, const QString &dbKey, const QVariant &defValue) :
    QObject(), name(name), dbKey(dbKey), defValue(defValue)
{
    if (lastCreatedCfgCategory == nullptr)
    {
        qCritical() << "No last created category while creating CfgEntry!";
        return;
    }

    parent = lastCreatedCfgCategory;
    parent->childs[name] = this;
}

CfgEntry::~CfgEntry()
{
}

QVariant CfgEntry::get() const
{
    if (cached)
        return cachedValue;

    QVariant cfgVal = CFG->get(parent->toString(), dbKey);
    if (!cfgVal.isValid())
    {
        if (defValueFunc)
            return (*defValueFunc)();
        else
            return defValue;
    }

    cachedValue = cfgVal;
    cached = true;
    return cfgVal;
}

QVariant CfgEntry::getDefultValue() const
{
    if (defValueFunc)
        return (*defValueFunc)();
    else
        return defValue;
}

void CfgEntry::set(const QVariant &value)
{
    CFG->set(parent->toString(), this->dbKey, value);
    cachedValue = value;
    cached = true;
    emit changed(value);
}

void CfgEntry::defineDefaultValueFunction(CfgEntry::DefaultValueProviderFunc func)
{
    defValueFunc = func;
}

QString CfgEntry::getFullDbKey() const
{
    return parent->toString()+"."+dbKey;
}

QString CfgEntry::getFullSymbolicKey() const
{
    return parent->toString()+"."+name;
}

CfgEntry::operator const CfgEntry*() const
{
    return this;
}

CfgEntry::operator QString() const
{
    return dbKey;
}
