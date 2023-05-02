#include "erdlinearrowitem.h"
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
    int weight = QGraphicsPathItem::isSelected() ? 3 : 1;
    QPen myPen = QPen(qApp->palette().text().color(), weight, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    painter->setPen(myPen);
    painter->setBrush(myPen.color());
    painter->drawPath(path());
}
