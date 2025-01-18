#include "erdchangelayout.h"


ErdChangeLayout::ErdChangeLayout() :
    ErdChange(Category::LAYOUT)
{
}

QString ErdChangeLayout::toDdl() const
{
    return QString();
}
