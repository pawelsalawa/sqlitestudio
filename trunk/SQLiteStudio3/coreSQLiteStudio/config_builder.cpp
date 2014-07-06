#include "config_builder.h"
#include "services/config.h"
#include <QDebug>

CfgCategory* lastCreatedCfgCategory = nullptr;
CfgMain* lastCreatedCfgMain = nullptr;
QList<CfgMain*> CfgMain::instances;

CfgMain::CfgMain(const QString& name, bool persistable) :
    name(name), persistable(persistable)
{
    lastCreatedCfgMain = this;
    instances << this;
}

CfgMain::~CfgMain()
{
    instances.removeOne(this);
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

QList<CfgMain*> CfgMain::getPersistableInstances()
{
    QList<CfgMain*> list;
    for (CfgMain* main : getInstances())
    {
        if (main->isPersistable())
            list << main;
    }
    return list;
}

QHash<QString, CfgCategory *> &CfgMain::getCategories()
{
    return childs;
}

void CfgMain::reset()
{
    for (CfgCategory* ctg : childs)
        ctg->reset();
}

void CfgMain::savepoint()
{
    for (CfgCategory* ctg : childs)
        ctg->savepoint();
}

void CfgMain::restore()
{
    for (CfgCategory* ctg : childs)
        ctg->restore();
}

void CfgMain::release()
{
    for (CfgCategory* ctg : childs)
        ctg->release();
}

bool CfgMain::isPersistable() const
{
    return persistable;
}

QString CfgMain::getName() const
{
    return name;
}

CfgCategory::CfgCategory(const QString &name)
{
    this->name = name;
    this->persistable = lastCreatedCfgMain->persistable;
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

void CfgCategory::reset()
{
    for (CfgEntry* entry : childs)
        entry->reset();
}

void CfgCategory::savepoint()
{
    for (CfgEntry* entry : childs)
        entry->savepoint();
}

void CfgCategory::restore()
{
    for (CfgEntry* entry : childs)
        entry->restore();
}

void CfgCategory::release()
{
    for (CfgEntry* entry : childs)
        entry->release();
}

CfgCategory::operator QString() const
{
    return name;
}

CfgEntry::CfgEntry(const CfgEntry& other) :
    QObject(), persistable(other.persistable), parent(other.parent), name(other.name), defValue(other.defValue),
    defValueFunc(other.defValueFunc)
{
}

CfgEntry::CfgEntry(const QString &name, const QVariant &defValue) :
    QObject(), name(name), defValue(defValue)
{
    if (lastCreatedCfgCategory == nullptr)
    {
        qCritical() << "No last created category while creating CfgEntry!";
        return;
    }

    parent = lastCreatedCfgCategory;
    persistable = parent->persistable;
    parent->childs[name] = this;
}

CfgEntry::~CfgEntry()
{
}

QVariant CfgEntry::get() const
{
    if (cached)
        return cachedValue;

    QVariant cfgVal;
    if (persistable)
        cfgVal = CFG->get(parent->toString(), name);

    cachedValue = cfgVal;
    cached = true;
    if (!persistable || !cfgVal.isValid())
    {
        if (defValueFunc)
            cachedValue = (*defValueFunc)();
        else
            cachedValue = defValue;

        return cachedValue;
    }

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
    if (persistable)
        CFG->set(parent->toString(), name, value);

    bool wasChanged = (value != cachedValue);

    cachedValue = value;
    cached = true;

    if (wasChanged)
        emit changed(value);
}

void CfgEntry::defineDefaultValueFunction(CfgEntry::DefaultValueProviderFunc func)
{
    defValueFunc = func;
}

QString CfgEntry::getFullKey() const
{
    return parent->toString()+"."+name;
}

void CfgEntry::reset()
{
    set(getDefultValue());
}

bool CfgEntry::isPersistable() const
{
    return persistable;
}

bool CfgEntry::isPersisted() const
{
    if (persistable)
        return !CFG->get(parent->toString(), name).isNull();

    return false;
}

void CfgEntry::savepoint()
{
    backup = get();
}

void CfgEntry::restore()
{
    set(backup);
}

void CfgEntry::release()
{
    backup.clear();
}

CfgEntry::operator CfgEntry*()
{
    return this;
}

CfgEntry::operator QString() const
{
    return name;
}
