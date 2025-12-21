#include "erditem.h"

ErdItem::~ErdItem()
{
}

bool ErdItem::isBeingDeleted() const
{
    return beingDeleted;
}

void ErdItem::markAsBeingDeleted()
{
    beingDeleted = true;
}
