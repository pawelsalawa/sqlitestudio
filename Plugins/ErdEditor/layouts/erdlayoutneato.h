#ifndef ERDLAYOUTNEATO_H
#define ERDLAYOUTNEATO_H

#include "erdlayout.h"

class ErdLayoutNeato : public ErdLayout
{
    public:
        ErdLayoutNeato();
        void arrange(QList<ErdEntity*> entities);
};

#endif // ERDLAYOUTNEATO_H
