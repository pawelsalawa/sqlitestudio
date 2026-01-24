#ifndef ERDLAYOUTFDP_H
#define ERDLAYOUTFDP_H

#include "erdlayout.h"

class ErdLayoutFdp : public ErdLayout
{
    public:
        ErdLayoutFdp();

        void arrange(QList<ErdEntity*> entities);
};

#endif // ERDLAYOUTFDP_H
