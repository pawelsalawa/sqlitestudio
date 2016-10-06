#include "committable.h"
#include <QDebug>

Committable::ConfirmFunction Committable::confirmFunc = nullptr;
QList<Committable*> Committable::instances;

Committable::Committable()
{
    instances << this;
}

Committable::~Committable()
{
    instances.removeOne(this);
}

void Committable::init(Committable::ConfirmFunction confirmFunc)
{
    Committable::confirmFunc = confirmFunc;
}

bool Committable::canQuit()
{
    if (!confirmFunc)
    {
        qCritical() << "No confirm function defined for Committable!";
        return true;
    }

    QList<Committable*> uncommittedInstances;
    for (Committable* c : instances)
    {
        if (c->isUncommitted())
            uncommittedInstances << c;
    }

    if (uncommittedInstances.size() == 0)
        return true;

    return confirmFunc(uncommittedInstances);
}
