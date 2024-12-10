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
        int getStartEntityRow() const;
        int getEndEntityRow() const;
        void setArrowType(ErdArrowItem::Type arrowType);

        /**
         * @brief Sets index of connection in the starting entity.
         * It's not a row number. It's more of a z-index of arrow relative to other arrows.
         * The square arrow type uses it to draw different horizontal distances for multiple connections.
         */
        void setIndexInStartEntity(int idx);

        /**
         * @brief Sets index of connection in the ending entity.
         * More details in the #setIndexInStartEntity();
         */
        void setIndexInEndEntity(int idx);

    private:
        static QPointF findThisPosAgainstOther(ErdEntity* thisEntity, int thisRow, const QPointF& otherPosition, ErdArrowItem::Side& entitySide);

        ErdEntity* startEntity = nullptr;
        ErdEntity* endEntity = nullptr;
        int startEntityRow = -1;
        int endEntityRow = -1;
        QPointF volatileEndPosition;
        ErdArrowItem* arrow = nullptr;
        ErdScene* scene = nullptr;
};

#endif // ERDCONNECTION_H
