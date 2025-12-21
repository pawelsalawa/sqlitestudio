#include "erdchangelayout.h"


ErdChangeLayout::ErdChangeLayout(const QString& description) :
    ErdChange(Category::LAYOUT, description)
{
}

QStringList ErdChangeLayout::toDdl()
{
    return QStringList();
}
