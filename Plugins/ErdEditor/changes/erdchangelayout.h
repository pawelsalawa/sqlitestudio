#ifndef ERDCHANGELAYOUT_H
#define ERDCHANGELAYOUT_H

#include "erdchange.h"


class ErdChangeLayout : public ErdChange
{
    public:
        ErdChangeLayout(const QString& description);

        QStringList toDdl();

    protected:
        QStringList getChangeDdl();
};

#endif // ERDCHANGELAYOUT_H
