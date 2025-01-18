#ifndef ERDCHANGELAYOUT_H
#define ERDCHANGELAYOUT_H

#include "erdchange.h"


class ErdChangeLayout : public ErdChange
{
    public:
        ErdChangeLayout();

        QString toDdl() const;
};

#endif // ERDCHANGELAYOUT_H
