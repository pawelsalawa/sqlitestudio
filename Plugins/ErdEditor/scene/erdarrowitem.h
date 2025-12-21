#ifndef ERDARROWITEM_H
#define ERDARROWITEM_H

#include "erditem.h"
#include <QGraphicsPathItem>

class ErdArrowItem : public ErdItem, public QGraphicsPathItem
{
    public:
        enum Side
        {
            LEFT,
            RIGHT,
            UNDEFINED
        };

        enum Type
        {
            STRAIGHT,
            CURVY,
            SQUARE
        };

        static ErdArrowItem* create(Type type);

        QPainterPath shape() const override;
        QRectF boundingRect() const override;
        bool isClickable() override;
        int type() const override;
        void setArrowIndexInStartEntity(int idx);
        void setArrowIndexInEndEntity(int idx);

        virtual void setPoints(const QLineF& line, Side startEntitySide, Side endEntitySide) = 0;

    protected:
        ErdArrowItem();

        void refreshArrowHead(qreal yDistance, qreal xDistance);

        QPainterPath linePath;
        int arrowItemType;
        QPointF startPoint;
        QPointF endPoint;
        Side startEntitySide;
        Side endEntitySide;
        QPolygonF arrowHead;
        qreal arrowSize = 10;
        int arrowIndexInStartEntity = 0;
        int arrowIndexInEndEntity = 0;
};

#endif // ERDARROWITEM_H
