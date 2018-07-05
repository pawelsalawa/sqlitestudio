#include "cfgmain.h"
#include "config_builder/cfgcategory.h"
#include "config_builder/cfgentry.h"
#include "common/global.h"
#include <QDebug>

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

void CfgMain::setValuesFromQVariant(const QVariant& cfgMainHash)
{
    QHash<QString, QVariant> mainHash = cfgMainHash.toHash();
    if (mainHash.isEmpty())
        return;

    QHash<QString, QVariant>::const_iterator mainIt = mainHash.begin();
    if (mainIt.key() != name)
    {
        qWarning() << "Tried to set CfgMain values from QVariant which does not have such main in its registry.";
        return;
    }

    QHash<QString, QVariant> categoriesHash = mainIt.value().toHash();
    QHash<QString, QVariant> entriesHash;
    QHash<QString, CfgEntry*> entries;
    for (QHash<QString, CfgCategory*>::const_iterator categoryIt = childs.begin(); categoryIt != childs.end(); categoryIt++)
    {
        if (!categoriesHash.contains(categoryIt.key()))
            continue;

        entriesHash = categoriesHash[categoryIt.key()].toHash();
        entries = categoryIt.value()->getEntries();
        for (QHash<QString, CfgEntry*>::const_iterator entryIt = entries.begin(); entryIt != entries.end(); entryIt++)
        {
            if (!entriesHash.contains(entryIt.key()))
                continue;

            entryIt.value()->set(entriesHash[entryIt.key()]);
        }
    }
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

QVariant CfgMain::toQVariant() const
{
    QHash<QString, QVariant> categoriesVariant;
    QHash<QString, QVariant> entriesVariant;
    QHash<QString, CfgEntry*> entries;
    for (QHash<QString, CfgCategory*>::const_iterator categoryIt = childs.begin(); categoryIt != childs.end(); categoryIt++)
    {
        entries = categoryIt.value()->getEntries();
        entriesVariant.clear();
        for (QHash<QString, CfgEntry*>::const_iterator entryIt = entries.begin(); entryIt != entries.end(); entryIt++)
            entriesVariant[entryIt.key()] = entryIt.value()->get();

        categoriesVariant[categoryIt.key()] = entriesVariant;
    }

    QHash<QString, QVariant> mainVariant;
    mainVariant[name] = categoriesVariant;
    return mainVariant;
}

CfgMain::operator CfgMain*()
{
    return this;
}
