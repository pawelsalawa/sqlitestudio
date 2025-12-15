#include "erdchangeregistry.h"
#include "erdchange.h"
#include <QDebug>

ErdChangeRegistry::ErdChangeRegistry(QObject *parent)
    : QObject{parent}
{

}

void ErdChangeRegistry::compact()
{
    // make the change list compact
}

void ErdChangeRegistry::addChange(ErdChange* change)
{
    // First discard any redo's if exist
    if (currentIndex < changes.size() - 1)
        changes.resize(currentIndex + 1);

    // Then add new change at the end
    changes << change;
    currentIndex++;
    notifyChangesUpdated();
}

int ErdChangeRegistry::getPandingChangesCount() const
{
    return currentIndex + 1;
}

QList<ErdChange*> ErdChangeRegistry::getEffectiveChanges() const
{
    return changes.sliced(0, currentIndex + 1);
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

void ErdChangeRegistry::notifyChangesUpdated()
{
    emit effectiveChangeCountUpdated(getPandingChangesCount());
}
