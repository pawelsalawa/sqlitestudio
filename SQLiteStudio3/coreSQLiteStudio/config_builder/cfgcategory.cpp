#include "cfgcategory.h"
#include "config_builder/cfgmain.h"
#include "config_builder/cfgentry.h"

CfgCategory* lastCreatedCfgCategory = nullptr;
extern CfgMain* lastCreatedCfgMain;

CfgCategory::CfgCategory(const CfgCategory &other) :
    name(other.name), title(other.title), persistable(other.persistable), childs(other.childs)
{
    lastCreatedCfgCategory = this;
    lastCreatedCfgMain->childs[name] = this;
    for (CfgEntry* entry : childs)
        entry->parent = this;
}

CfgCategory::CfgCategory(const QString &name, const QString &title) :
    name(name), title(title)
{
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

QString CfgCategory::getTitle() const
{
    return title;
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
