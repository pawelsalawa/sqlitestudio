#include "erdcurvyarrowitem.h"
#include "style.h"
#include <QPen>
#include <QPalette>
#include <QPainter>
#include <QApplication>
#include <QDebug>

ErdCurvyArrowItem::ErdCurvyArrowItem() :
    ErdArrowItem()
{
}

void ErdCurvyArrowItem::setPoints(const QLineF& line, Side startSide, Side endSide)
{
    this->startEntitySide = startSide;
    this->endEntitySide = endSide;
    startPoint = line.p1();
    endPoint = line.p2();

    if (endSide == ErdArrowItem::UNDEFINED)
    {
        if (startSide == LEFT && (startPoint.x() - 10) < endPoint.x())
            endSide = LEFT;
        else if (startSide == RIGHT && (startPoint.x() + 10) >= endPoint.x())
            endSide = RIGHT;
    }

    if (startSide != endSide)
    {
        int sideSign = startSide == LEFT ? -1 : 1;
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
        int sideSign = endSide == LEFT ? -1 : 1;
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
    QPen myPen = QPen(STYLE->standardPalette().text().color(), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    painter->setPen(myPen);
    painter->drawPath(path());

    painter->setBrush(myPen.color());
    painter->drawPolygon(arrowHead);

    if (isSelected())
    {
        QPen outlinePen;
        outlinePen.setColor(STYLE->standardPalette().highlight().color());
        outlinePen.setStyle(Qt::DotLine);
        outlinePen.setWidth(3);

        painter->setBrush(Qt::NoBrush);
        painter->setPen(outlinePen);
        painter->drawPath(path());

        painter->setBrush(outlinePen.color());
        painter->drawPolygon(arrowHead);
    }
}

