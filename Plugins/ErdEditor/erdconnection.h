#ifndef ERDCONNECTION_H
#define ERDCONNECTION_H

#include <QObject>
#include <QPointF>
#include "erdarrowitem.h"

class ErdEntity;
class ErdScene;
class ErdCurvyArrowItem;
class QGraphicsScene;

class ErdConnection
{
    public:
        ErdConnection(ErdEntity* startEntity, const QPointF& endPos, ErdArrowItem::Type arrowType);
        ErdConnection(ErdEntity* startEntity, int startRow, ErdEntity* endEntity, int endRow, ErdArrowItem::Type arrowType);
        virtual ~ErdConnection();

        void addToScene(ErdScene* scene);
        void updatePosition(const QPointF& endPos);
        void finalizeConnection(ErdEntity* entity, const QPointF& endPos);
        bool isFinalized() const;
        void refreshPosition();
        ErdEntity* getStartEntity() const;
        ErdEntity* getEndEntity() const;
        void setArrowType(ErdArrowItem::Type arrowType);

    private:
        static QPointF findThisPosAgainstOther(ErdEntity* thisEntity, int thisRow, const QPointF& otherPosition, ErdArrowItem::Side& entitySide);

        ErdEntity* startEntity = nullptr;
        ErdEntity* endEntity = nullptr;
        int startEntityRow = -1;
        int endEntityRow = -1;
        QPointF volatileEndPosition;
        ErdArrowItem* arrow = nullptr;
        ErdScene* scene = scene;
};

#endif // ERDCONNECTION_H
