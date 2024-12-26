#ifndef ERDCHANGE_H
#define ERDCHANGE_H

#include <QString>

class ErdChange
{
    public:
        ErdChange();

        virtual QString toDdl() const = 0;
};

#endif // ERDCHANGE_H
