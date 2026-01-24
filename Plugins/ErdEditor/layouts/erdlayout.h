#ifndef ERDLAYOUT_H
#define ERDLAYOUT_H

#include <QList>
#include <QRectF>

class ErdEntity;

class ErdLayout
{
    public:
        virtual ~ErdLayout() {}

        virtual void arrange(QList<ErdEntity*> entities) = 0;
        static QRectF computeBoundingBox(const QList<ErdEntity*>& entities);
};

#endif // ERDLAYOUT_H
