#include "erdarrowitem.h"
#include "erdcurvyarrowitem.h"
#include "erdlinearrowitem.h"
#include "erdsquarearrowitem.h"
#include <QGraphicsDropShadowEffect>
#include <QPen>
#include <QtMath>
#include <QDebug>

ErdArrowItem::ErdArrowItem() :
    QGraphicsPathItem()
{
    setZValue(1);

    QGraphicsDropShadowEffect* effect = new QGraphicsDropShadowEffect();
    effect->setBlurRadius(20);
    effect->setOffset(4, 4);
    effect->setColor(QColor(0, 0, 0, 128));
    setGraphicsEffect(effect);
}

void ErdArrowItem::refreshArrowHead(qreal yDistance, qreal xDistance)
{
    double angle = std::atan2(yDistance, xDistance);
    QPointF arrowP1 = endPoint + QPointF(sin(angle + M_PI / 3) * arrowSize,
                                           cos(angle + M_PI / 3) * arrowSize);
    QPointF arrowP2 = endPoint + QPointF(sin(angle + M_PI - M_PI / 3) * arrowSize,
                                           cos(angle + M_PI - M_PI / 3) * arrowSize);

    arrowHead.clear();
    arrowHead << endPoint << arrowP1 << arrowP2;
}

ErdArrowItem* ErdArrowItem::create(Type type)
{
    ErdArrowItem* item = nullptr;
    switch (type)
    {
        case STRAIGHT:
            item = new ErdLineArrowItem();
            break;
        case CURVY:
            item = new ErdCurvyArrowItem();
            break;
        case SQUARE:
            item = new ErdSquareArrowItem();
            break;
        default:
            qCritical() << "Unsupported ERD arrow item type:" << static_cast<int>(type);
            return nullptr;
    }
    item->arrowItemType = type;
    return item;
}

QPainterPath ErdArrowItem::shape() const
{
    QPainterPath p = QGraphicsPathItem::shape();
    p.addPolygon(arrowHead);
    return p;
}

QRectF ErdArrowItem::boundingRect() const
{
    qreal extra = (pen().width() + arrowSize) / 2.0;
    return QGraphicsPathItem::boundingRect().adjusted(-extra, -extra, extra, extra);
}

bool ErdArrowItem::isClickable()
{
    return flags().testFlag(QGraphicsItem::ItemIsSelectable);
}

int ErdArrowItem::type() const
{
    return arrowItemType;
}
