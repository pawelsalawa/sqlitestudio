#include "erdchangeregistry.h"
#include "erdchange.h"
#include <QDebug>

ErdChangeRegistry::ErdChangeRegistry(QObject *parent)
    : QObject{parent}
{

}

QList<ErdChange*> ErdChangeRegistry::compactedEffectiveChanges()
{
    // TODO
    /*
     * Changes can be compacted only at the final commit stage, because we need to maintain the
     * undo/redo capability until then.
     *
     * TODO planned compacting optimalizations:
     *
     * - after creating table X and deleting the same table X - remove both changes
     * - after modifying same table multiple times - generate single aggregated change between 1st and last
     * - if multiple editions reduce to no changes - remove all these changes entirely
     * - multiple changes and drop of table at the end - just drop the end table, but only if there are
     *   no referring views, otherwise we won't be able to reflect actual changes to views
     */
    return QList<ErdChange*>{};
}

void ErdChangeRegistry::addChange(ErdChange* change)
{
    // First discard any redo's if exist
    if (currentIndex < changes.size() - 1)
    {
        for (auto chg : changes.mid(currentIndex + 1))
            delete chg;

        changes.resize(currentIndex + 1);
    }

    // Then add new change at the end
    changes << change;
    currentIndex++;
    notifyChangesUpdated();
}

int ErdChangeRegistry::getPendingChangesCount() const
{
    return getPendingChanges().size();
}

QList<ErdChange*> ErdChangeRegistry::getPendingChanges(bool includeNonDdl) const
{
    auto result = changes.sliced(0, currentIndex + 1);
    if (includeNonDdl)
        return result;

    return result | FILTER(chg, {return chg->isDdlChange();});
}

ErdChange* ErdChangeRegistry::undo()
{
    ErdChange* change = peekUndo();
    if (change)
    {
        currentIndex--;
        notifyChangesUpdated();
    }

    return change;
}

ErdChange* ErdChangeRegistry::redo()
{
    ErdChange* change = peekRedo();
    if (change)
    {
        currentIndex++;
        notifyChangesUpdated();
    }

    return change;
}

ErdChange* ErdChangeRegistry::peekUndo() const
{
    if (currentIndex == -1)
        return nullptr;

    return changes[currentIndex];
}

ErdChange* ErdChangeRegistry::peekRedo() const
{
    if (currentIndex >= changes.size())
        return nullptr;

    return changes[currentIndex + 1];
}

bool ErdChangeRegistry::isUndoAvailable() const
{
    return currentIndex >= 0;
}

bool ErdChangeRegistry::isRedoAvailable() const
{
    return changes.size() > 0 && currentIndex < (changes.size() - 1);
}

void ErdChangeRegistry::clear()
{
    for (ErdChange*& chg : changes)
        delete chg;

    changes.clear();
}

void ErdChangeRegistry::notifyChangesUpdated()
{
    emit effectiveChangeCountUpdated(getPendingChangesCount(), isUndoAvailable(), isRedoAvailable());
}
