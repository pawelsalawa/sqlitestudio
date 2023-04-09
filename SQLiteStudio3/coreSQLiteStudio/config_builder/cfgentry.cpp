#include "cfgentry.h"
#include "config_builder/cfgmain.h"
#include "config_builder/cfgcategory.h"
#include "services/config.h"
#include <QDebug>

extern CfgCategory* lastCreatedCfgCategory;

CfgEntry::CfgEntry(const CfgEntry& other) :
    QObject(), persistable(other.persistable), parent(other.parent), name(other.name), defValue(other.defValue),
    title(other.title), defValueFunc(other.defValueFunc)
{
    connect(this, SIGNAL(changed(QVariant)), parent, SLOT(handleEntryChanged()));
}

CfgEntry::CfgEntry(const QString &name, const QVariant &defValue, const QString &title) :
    QObject(), name(name), defValue(defValue), title(title)
{
    if (lastCreatedCfgCategory == nullptr)
    {
        qCritical() << "No last created category while creating CfgEntry!";
        return;
    }

    parent = lastCreatedCfgCategory;
    persistable = parent->persistable;
    parent->childs[name] = this;
    connect(this, SIGNAL(changed(QVariant)), parent, SLOT(handleEntryChanged()));
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

QVariant CfgEntry::getDefaultValue() const
{
    if (defValueFunc)
        return (*defValueFunc)();
    else
        return defValue;
}

void CfgEntry::set(const QVariant &value)
{
    bool doPersist = persistable && !transaction;
    bool wasChanged = (value != cachedValue);

    if (doPersist && wasChanged)
        CFG->set(parent->toString(), name, value);

    if (wasChanged)
        cachedValue = value;

    cached = true;

    if (wasChanged)
        emit changed(value);

    if (doPersist)
        emit persisted(value);
}

void CfgEntry::defineDefaultValueFunction(CfgEntry::DefaultValueProviderFunc func)
{
    defValueFunc = func;
}

QString CfgEntry::getFullKey() const
{
    return parent->toString()+"."+name;
}

QString CfgEntry::getTitle() const
{
    return title;
}

QString CfgEntry::getName() const
{
    return name;
}

void CfgEntry::translateTitle()
{
    // This needs to be "QObject::tr" and not just "tr". See CfgCategory::translateTitle() for details.
    title = QObject::tr(title.toUtf8().constData());
}

void CfgEntry::reset()
{
    set(getDefaultValue());
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

void CfgEntry::savepoint(bool transaction)
{
    backup = get();
    this->transaction = transaction;
}

void CfgEntry::begin()
{
    savepoint(true);
}

void CfgEntry::restore()
{
    cachedValue = backup;
    cached = true;
    transaction  = false;
}

void CfgEntry::release()
{
    backup.clear();
    if (transaction)
    {
        transaction = false;
        if (cached)
        {
            QVariant valueToSet = cachedValue;
            cachedValue = QVariant();
            cached = false;
            set(valueToSet);
        }
    }

}

void CfgEntry::commit()
{
    if (!transaction)
        return;

    release();
}

void CfgEntry::rollback()
{
    if (!transaction)
        return;

    restore();
}

CfgCategory* CfgEntry::getCategory() const
{
    return parent;
}

CfgMain* CfgEntry::getMain() const
{
    return parent->getMain();
}

CfgEntry::operator CfgEntry*()
{
    return this;
}

CfgEntry::operator QString() const
{
    return name;
}
