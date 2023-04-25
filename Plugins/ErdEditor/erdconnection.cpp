#include "erdconnection.h"
#include "erdarrowitem.h"
#include "erdentity.h"
#include <QDebug>

ErdConnection::ErdConnection(ErdEntity* startEntity, const QPointF& endPos) :
    startEntity(startEntity), volatileEndPosition(endPos)
{
    startEntityRow = startEntity->rowIndexAt(endPos);
    arrow = new ErdArrowItem(calcPositions());
    startEntity->addConnection(this);
}

ErdConnection::ErdConnection(ErdEntity* startEntity, int startRow, ErdEntity* endEntity, int endRow) :
    startEntity(startEntity), endEntity(endEntity), startEntityRow(startRow), endEntityRow(endRow)
{
    arrow = new ErdArrowItem(calcPositions());
    startEntity->addConnection(this);
    endEntity->addConnection(this);
}

ErdConnection::~ErdConnection()
{
    startEntity->removeConnection(this);
    if (endEntity)
        endEntity->removeConnection(this);

    delete arrow;
}

ErdArrowItem* ErdConnection::getArrow() const
{
    return arrow;
}

void ErdConnection::updatePosition(const QPointF& endPos)
{
    volatileEndPosition = endPos;
    arrow->setLine(calcPositions());
}

void ErdConnection::finalizeConnection(ErdEntity* entity, const QPointF& endPos)
{
    endEntity = entity;
    endEntityRow = entity->rowIndexAt(endPos);
    endEntity->addConnection(this);
    arrow->setFlag(QGraphicsItem::ItemIsSelectable, true);
    arrow->setLine(calcPositions());
}

bool ErdConnection::isFinalized() const
{
    return endEntity != nullptr;
}

void ErdConnection::refreshPosition()
{
    arrow->setLine(calcPositions());
}

QLineF ErdConnection::calcPositions()
{
    QPointF srcPos;
    QPointF trgPos;
    if (endEntity)
    {
        QPointF endPos = endEntity->pos();
        srcPos = findThisPosAgainstOther(startEntity, startEntityRow, endPos);
        trgPos = findThisPosAgainstOther(endEntity, endEntityRow, srcPos);
    }
    else
    {
        srcPos = findThisPosAgainstOther(startEntity, startEntityRow, volatileEndPosition);
        trgPos = volatileEndPosition;
    }
    return QLineF(trgPos, srcPos);
}

QPointF ErdConnection::findThisPosAgainstOther(ErdEntity* thisEntity, int thisRow, const QPointF& otherPosition)
{
    QPointF entityPos = thisEntity->pos();
    QRectF rowRect = (thisRow > -1) ? thisEntity->rowRect(thisRow) : QRectF(0, 0, 0, 0);
    QRectF entityRect = thisEntity->rect();

    QPointF pos = entityPos + rowRect.topLeft() + QPointF(0, rowRect.height() / 2);
    if (otherPosition.x() > (entityPos.x() + (entityRect.width()) / 2))
        pos.rx() += entityRect.width();

    return pos;
}

ErdEntity* ErdConnection::getEndEntity() const
{
    return endEntity;
}

ErdEntity* ErdConnection::getStartEntity() const
{
    return startEntity;
}

