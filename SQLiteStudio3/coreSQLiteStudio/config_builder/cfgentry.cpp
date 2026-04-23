#include "cfgentry.h"
#include "common/utils.h"
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
    if (persistable && isDependencySatisfied())
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

    if (doPersist && wasChanged && isDependencySatisfied())
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
        return !isNull(CFG->get(parent->toString(), name));

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

bool CfgEntry::isDependencySatisfied() const
{
    if (!dependencyDef)
        return true;

    if (!resolvedDependency)
    {
        if (dependencyDef->categoryName.isNull())
        {
            resolvedDependency = getCategory()->getEntryByName(dependencyDef->entryName);
        }
        else
        {
            CfgCategory* cat = getMain()->getCategoryByName(dependencyDef->categoryName);
            if (cat)
                resolvedDependency = cat->getEntryByName(dependencyDef->entryName);
        }
    }

    if (!resolvedDependency)
    {
        return true;
    }

    return resolvedDependency->get().toBool();
}

const CfgEntry::CfgDependency* CfgEntry::getDependencyDefinition() const
{
    return dependencyDef;
}

QString CfgEntry::getDependencyFullKey() const
{
    if (!dependencyDef)
        return QString();

    static_qstring(keyTpl, "%1.%2");
    return keyTpl.arg(
            dependencyDef->categoryName.isNull() ? getCategory()->toString() : dependencyDef->categoryName,
            dependencyDef->entryName
        );
}

CfgEntry::operator QString() const
{
    return name;
}

CfgEntry::CfgDependency::CfgDependency(const QString &key) :
    CfgDependency(key, true, false)
{
}

CfgEntry::CfgDependency::CfgDependency(const QString& key, const QVariant& expectedValue, bool hide)
{
    QStringList parts = key.split(".");
    if (parts.size() >= 2)
    {
        categoryName = parts[0];
        entryName = parts[1];
    }
    else if (parts.size() == 1)
    {
        entryName = parts[0];
    }

    this->expectedValue = expectedValue;
    this->hideWhenUnsatisfied = hide;
}
