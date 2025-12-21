#include "erdlinearrowitem.h"
#include "style.h"
#include <QPen>
#include <QPalette>
#include <QPainter>
#include <QApplication>
#include <QGraphicsDropShadowEffect>

ErdLineArrowItem::ErdLineArrowItem() :
    ErdArrowItem()
{
}

void ErdLineArrowItem::setPoints(const QLineF& line, Side startEntitySide, Side endEntitySide)
{
    this->startEntitySide = startEntitySide;
    this->endEntitySide = endEntitySide;
    startPoint = line.p1();
    endPoint = line.p2();

    refreshArrowHead(line.dy(), -line.dx());

    QPainterPath thePath;
    thePath.moveTo(startPoint);
    thePath.lineTo(endPoint);
    thePath.closeSubpath();
    thePath.addPolygon(arrowHead);
    setPath(thePath);
}

void ErdLineArrowItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    QPen myPen = QPen(STYLE->standardPalette().text().color(), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    painter->setPen(myPen);
    painter->setBrush(myPen.color());
    painter->drawPath(path());


    if (isSelected())
    {
        QPen outlinePen;
        outlinePen.setColor(STYLE->standardPalette().highlight().color());
        outlinePen.setStyle(Qt::DotLine);
        outlinePen.setWidth(3);

        painter->setBrush(outlinePen.color());
        painter->setPen(outlinePen);
        painter->drawPath(path());
    }
}
