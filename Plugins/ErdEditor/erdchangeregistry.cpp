#include "erdchangeregistry.h"

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
    changes << change;
}
