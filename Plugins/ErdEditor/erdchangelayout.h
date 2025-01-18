#ifndef ERDCHANGELAYOUT_H
#define ERDCHANGELAYOUT_H

#include "erdchange.h"


class ErdChangeLayout : public ErdChange
{
    public:
        ErdChangeLayout();

        QStringList toDdl();
};

#endif // ERDCHANGELAYOUT_H
