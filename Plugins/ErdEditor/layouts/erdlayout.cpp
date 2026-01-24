#include "layouts/erdlayout.h"
#include "scene/erdentity.h"

QRectF ErdLayout::computeBoundingBox(const QList<ErdEntity*>& entities)
{
    QRectF bbox;
    for (ErdEntity* e : entities)
    {
        if (!e)
            continue;

        bbox |= e->mapToScene(e->boundingRect()).boundingRect();
    }
    return bbox;
}
