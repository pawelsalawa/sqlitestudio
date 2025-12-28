#include "erdchangecomposite.h"

ErdChangeComposite::ErdChangeComposite(const QString& description) :
    ErdChange(Category::ENTITY_CHANGE, description, true)
{
}

ErdChangeComposite::ErdChangeComposite(QList<ErdChange*> changes, const QString& description) :
    ErdChange(Category::ENTITY_CHANGE, description, true), changes(changes)
{
}

void ErdChangeComposite::addChange(ErdChange* change)
{
    changes << change;
}

ErdChangeComposite &ErdChangeComposite::operator<<(ErdChange* change)
{
    changes << change;
    return *this;
}

ErdChangeComposite &ErdChangeComposite::operator+=(const QList<ErdChange*>& changeList)
{
    changes += changeList;
    return *this;
}

QStringList ErdChangeComposite::getChangeDdl()
{
    QStringList results;
    for (ErdChange*& chg : changes)
        results += chg->toDdl(true);

    return results;
}

QList<ErdChange*> ErdChangeComposite::getChanges() const
{
    return changes;
}

QStringList ErdChangeComposite::getUndoDdl()
{
    if (!changes.isEmpty())
        return changes.first()->getUndoDdl();

    return ErdChange::getUndoDdl();
}

QStringList ErdChangeComposite::provideUndoEntitiesToRefresh() const
{
    QStringList results;
    for (auto chg : reverse(changes))
        results += chg->provideUndoEntitiesToRefresh();

    return results;
}
