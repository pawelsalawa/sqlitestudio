#include "erdcurvyarrowitem.h"
#include <QPen>
#include <QPalette>
#include <QPainter>
#include <QApplication>
#include <QDebug>

ErdCurvyArrowItem::ErdCurvyArrowItem() :
    ErdArrowItem()
{
}

void ErdCurvyArrowItem::setPoints(const QLineF& line, Side startEntitySide, Side endEntitySide)
{
    this->startEntitySide = startEntitySide;
    this->endEntitySide = endEntitySide;
    startPoint = line.p1();
    endPoint = line.p2();

    if (startEntitySide != endEntitySide)
    {
        int sideSign = startEntitySide == LEFT ? -1 : 1;
        if (endEntitySide == ErdArrowItem::UNDEFINED)
            sideSign = startPoint.x() < endPoint.x() ? -1 : 1;

        refreshArrowHead(0, -sideSign);

        QPointF controlPoint1 = startPoint + QPointF(50 * sideSign, 0);
        QPointF controlPoint2 = endPoint + QPointF(-50 * sideSign, 0);

        QPointF middle = (startPoint + endPoint) / 2;
        QPointF startLinePoint = startPoint + QPoint(10 * sideSign, 0);
        QPointF endLinePoint = endPoint + QPoint(-10 * sideSign, 0);

        QPainterPath thePath;
        thePath.moveTo(startPoint);
        thePath.lineTo(startLinePoint);
        thePath.quadTo(controlPoint1, middle);
        thePath.quadTo(controlPoint2, endLinePoint);
        thePath.lineTo(endPoint);
        setPath(thePath);
    }
    else
    {
        int sideSign = endEntitySide == LEFT ? -1 : 1;
        refreshArrowHead(0, sideSign);

        QPointF controlPoint1 = startPoint + QPointF(50 * sideSign, 0);
        QPointF controlPoint2 = endPoint + QPointF(50 * sideSign, 0);

        QPointF startLinePoint = startPoint + QPoint(10 * sideSign, 0);
        QPointF endLinePoint = endPoint + QPoint(10 * sideSign, 0);

        QPainterPath thePath;
        thePath.moveTo(startPoint);
        thePath.lineTo(startLinePoint);
        thePath.cubicTo(controlPoint1, controlPoint2, endLinePoint);
        thePath.lineTo(endPoint);
        setPath(thePath);
    }
}

void ErdCurvyArrowItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    int weight = QGraphicsPathItem::isSelected() ? 3 : 1;
    QPen myPen = QPen(qApp->palette().text().color(), weight, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    painter->setPen(myPen);
    painter->drawPath(path());
    painter->setBrush(myPen.color());
    painter->drawPolygon(arrowHead);
}

