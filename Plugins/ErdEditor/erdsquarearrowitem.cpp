#include "erdsquarearrowitem.h"
#include "style.h"
#include <QPen>
#include <QPalette>
#include <QPainter>
#include <QApplication>
#include <QDebug>

ErdSquareArrowItem::ErdSquareArrowItem() :
    ErdArrowItem()
{
}

void ErdSquareArrowItem::setPoints(const QLineF& line, Side startSide, Side endSide)
{
    this->startEntitySide = startSide;
    this->endEntitySide = endSide;
    startPoint = line.p1();
    endPoint = line.p2();

    if (endSide == ErdArrowItem::UNDEFINED)
    {
        if (startSide == LEFT && (startPoint.x() - INITIAL_GAP) < endPoint.x())
            endSide = LEFT;
        else if (startSide == RIGHT && (startPoint.x() + INITIAL_GAP) >= endPoint.x())
            endSide = RIGHT;
    }

    int startSideSign = startSide == LEFT ? -1 : 1;
    int endSideSign = startSideSign;
    int verticalSign = endPoint.y() > startPoint.y() ? -1 : 1;
    if (startSide == endSide)
        endSideSign *= -1;

    QPointF startLinePoint = startPoint + QPointF(INITIAL_GAP * startSideSign, 0);
    QPointF endLinePoint = endPoint + QPointF(-INITIAL_GAP * endSideSign, 0);
    if ((startLinePoint.x() > endLinePoint.x()) == (endSideSign < 0))
        verticalSign = 0;

    QPointF verticalLinePoint = QPointF(startLinePoint.x(), endLinePoint.y() + verticalSign * INITIAL_GAP);
    QPointF finalHorizontalLinePoint = QPointF(endLinePoint.x(), endLinePoint.y() + verticalSign * INITIAL_GAP);
    QPointF finalVerticalLinePoint = QPointF(endLinePoint.x(), endLinePoint.y());

    QPainterPath thePath;
    thePath.moveTo(startPoint);
    thePath.lineTo(startLinePoint);
    thePath.lineTo(verticalLinePoint);
    thePath.lineTo(finalHorizontalLinePoint);
    thePath.lineTo(finalVerticalLinePoint);
    thePath.lineTo(endLinePoint);
    thePath.lineTo(endPoint);

    refreshArrowHead(0, -endSideSign);

    setPath(thePath);
}

void ErdSquareArrowItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
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

