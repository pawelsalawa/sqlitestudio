#include "erdchangelayout.h"


ErdChangeLayout::ErdChangeLayout(const QString& description) :
    ErdChange(Category::LAYOUT, description)
{
}

QStringList ErdChangeLayout::provideUndoEntitiesToRefresh() const
{
    return {};
}

QStringList ErdChangeLayout::getChangeDdl()
{
    return {};
}


