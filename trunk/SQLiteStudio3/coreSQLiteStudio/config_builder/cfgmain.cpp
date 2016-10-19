#include "cfgmain.h"
#include "config_builder/cfgcategory.h"
#include "config_builder/cfgentry.h"
#include "common/global.h"

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

CfgCategory* CfgMain::getCategoryByName(const QString &name)
{
    for (CfgMain* cfg : getInstances())
    {
        if (!cfg->childs.contains(name))
            continue;

        return cfg->childs[name];
    }
    return nullptr;
}

CfgEntry *CfgMain::getEntryByName(const QString &categoryName, const QString &name)
{
    CfgCategory* cat = getCategoryByName(categoryName);
    if (!cat)
        return nullptr;

    return cat->getEntryByName(name);
}

CfgEntry *CfgMain::getEntryByPath(const QString &path)
{
    QStringList sp = path.split(".");
    if (sp.size() != 2)
        return nullptr;

    return getEntryByName(sp[0], sp[1]);
}

QHash<QString, CfgCategory *> &CfgMain::getCategories()
{
    return childs;
}

void CfgMain::translateTitle()
{
    title = QObject::tr(title.toUtf8().constData());
    for (CfgCategory* ctg : childs)
        ctg->translateTitle();
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

QStringList CfgMain::getPaths() const
{
    static_qstring(tpl, "%1.%2");
    QStringList paths;
    for (CfgCategory* cat : childs.values())
    {
        for (const QString& entry : cat->getEntries().keys())
            paths << tpl.arg(cat->toString(), entry);
    }
    return paths;
}

QList<CfgEntry *> CfgMain::getEntries() const
{
    QList<CfgEntry*> entries;
    for (CfgCategory* cat : childs.values())
        entries += cat->getEntries().values();

    return entries;
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
