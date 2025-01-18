#include "erdchangelayout.h"


ErdChangeLayout::ErdChangeLayout() :
    ErdChange(Category::LAYOUT)
{
}

QStringList ErdChangeLayout::toDdl()
{
    return QStringList();
}
