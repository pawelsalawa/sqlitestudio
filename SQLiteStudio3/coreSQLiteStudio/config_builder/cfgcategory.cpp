#include "cfgcategory.h"
#include "config_builder/cfgmain.h"
#include "config_builder/cfgentry.h"

CfgCategory* lastCreatedCfgCategory = nullptr;
extern CfgMain* lastCreatedCfgMain;

CfgCategory::CfgCategory(const CfgCategory &other) :
    QObject(), name(other.name), title(other.title), persistable(other.persistable), childs(other.childs)
{
    lastCreatedCfgCategory = this;
    lastCreatedCfgMain->childs[name] = this;
    cfgParent = lastCreatedCfgMain;
    for (CfgEntry*& entry : childs)
        entry->parent = this;
}

CfgCategory::CfgCategory(const QString &name, const QString &title) :
    name(name), title(title)
{
    this->persistable = lastCreatedCfgMain->persistable;
    lastCreatedCfgCategory = this;
    cfgParent = lastCreatedCfgMain;
    lastCreatedCfgMain->childs[name] = this;
}

CfgEntry *CfgCategory::getEntryByName(const QString& name)
{
    if (childs.contains(name))
        return childs[name];

    return nullptr;
}

QString CfgCategory::toString() const
{
    return name;
}

QHash<QString, CfgEntry *> &CfgCategory::getEntries()
{
    return childs;
}

void CfgCategory::translateTitle()
{
    // This needs to be "QObject::tr" and not just "tr", because this guarantees proper message context for retranslating
    // titles for objects initialized in global scope (as CfgCategories are).
    title = QObject::tr(title.toUtf8().constData());
    for (CfgEntry*& entry : childs)
        entry->translateTitle();

}

void CfgCategory::reset()
{
    for (CfgEntry*& entry : childs)
        entry->reset();
}

void CfgCategory::savepoint(bool transaction)
{
    for (CfgEntry*& entry : childs)
        entry->savepoint(transaction);
}

void CfgCategory::restore()
{
    for (CfgEntry*& entry : childs)
        entry->restore();
}

void CfgCategory::release()
{
    for (CfgEntry*& entry : childs)
        entry->release();
}

void CfgCategory::commit()
{
    release();
}

void CfgCategory::rollback()
{
    for (CfgEntry*& entry : childs)
        entry->rollback();
}

void CfgCategory::begin()
{
    savepoint(true);
}

QString CfgCategory::getTitle() const
{
    return title;
}

CfgMain*CfgCategory::getMain() const
{
    return cfgParent;
}

CfgCategory::operator CfgCategory *()
{
    return this;
}

void CfgCategory::handleEntryChanged()
{
    emit changed(dynamic_cast<CfgEntry*>(sender()));
}

CfgCategory::operator QString() const
{
    return name;
}
