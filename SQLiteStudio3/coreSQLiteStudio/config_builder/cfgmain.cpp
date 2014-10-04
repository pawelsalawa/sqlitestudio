#include "cfgmain.h"
#include "config_builder/cfgcategory.h"
#include "config_builder/cfgentry.h"

CfgMain* lastCreatedCfgMain = nullptr;
QList<CfgMain*>* CfgMain::instances = nullptr;

CfgMain::CfgMain(const QString& name, bool persistable, const char *metaName, const QString &title) :
    name(name), metaName(metaName), title(title), persistable(persistable)
{
    lastCreatedCfgMain = this;

    if (!instances)
        instances = new QList<CfgMain*>();

    *instances << this;
}

CfgMain::~CfgMain()
{
    if (!instances)
        instances = new QList<CfgMain*>();

    instances->removeOne(this);
}

void CfgMain::staticInit()
{
    qRegisterMetaType<CfgMain*>("CfgMain*");
    qRegisterMetaType<CfgCategory*>("CfgCategory*");
    qRegisterMetaType<CfgEntry*>("CfgEntry*");
}

QList<CfgMain*> CfgMain::getInstances()
{
    if (!instances)
        instances = new QList<CfgMain*>();

    return *instances;
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

void CfgMain::savepoint(bool transaction)
{
    for (CfgCategory* ctg : childs)
        ctg->savepoint(transaction);
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

void CfgMain::begin()
{
    savepoint(true);
}

void CfgMain::commit()
{
    release();
}

void CfgMain::rollback()
{
    restore();
}

bool CfgMain::isPersistable() const
{
    return persistable;
}

QString CfgMain::getName() const
{
    return name;
}

const char *CfgMain::getMetaName() const
{
    return metaName;
}

QString CfgMain::getTitle() const
{
    return title;
}

CfgMain::operator CfgMain*()
{
    return this;
}
