#include "erdconnection.h"
#include "erdlinearrowitem.h"
#include "erdcurvyarrowitem.h"
#include "erdentity.h"
#include "erdscene.h"
#include "erdeditorplugin.h"
#include <QDebug>
#include <QGraphicsScene>

ErdConnection::ErdConnection(ErdEntity* startEntity, const QPointF& endPos, ErdArrowItem::Type arrowType) :
    startEntity(startEntity), volatileEndPosition(endPos)
{
    startEntityRow = startEntity->rowIndexAt(endPos);
    arrow = ErdArrowItem::create(arrowType);
    arrow->setFlag(QGraphicsItem::ItemIsMovable, false);
    refreshPosition();
    startEntity->addConnection(this);
}

ErdConnection::ErdConnection(ErdEntity* startEntity, int startRow, ErdEntity* endEntity, int endRow, ErdArrowItem::Type arrowType) :
    startEntity(startEntity), endEntity(endEntity), startEntityRow(startRow), endEntityRow(endRow)
{
    arrow = ErdArrowItem::create(arrowType);
    refreshPosition();
    arrow->setFlag(QGraphicsItem::ItemIsMovable, false);
    arrow->setFlag(QGraphicsItem::ItemIsSelectable, true);
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

void ErdConnection::addToScene(ErdScene* scene)
{
    this->scene = scene;
    scene->addItem(arrow);
}

void ErdConnection::updatePosition(const QPointF& endPos)
{
    volatileEndPosition = endPos;
    refreshPosition();
}

void ErdConnection::finalizeConnection(ErdEntity* entity, const QPointF& endPos)
{
    endEntity = entity;
    endEntityRow = entity->rowIndexAt(endPos);
    endEntity->addConnection(this);
    arrow->setFlag(QGraphicsItem::ItemIsSelectable, true);
    startEntity->updateConnectionIndexes();
    endEntity->updateConnectionIndexes();
    refreshPosition();
}

bool ErdConnection::isFinalized() const
{
    return endEntity != nullptr;
}

void ErdConnection::refreshPosition()
{
    QPointF srcPos;
    QPointF trgPos;
    ErdArrowItem::Side startEntitySide = ErdArrowItem::UNDEFINED;
    ErdArrowItem::Side endEntitySide = ErdArrowItem::UNDEFINED;
    if (endEntity)
    {
        QPointF endPos = endEntity->pos();
        srcPos = findThisPosAgainstOther(startEntity, startEntityRow, endPos, startEntitySide);
        trgPos = findThisPosAgainstOther(endEntity, endEntityRow, srcPos, endEntitySide);
    }
    else
    {
        srcPos = findThisPosAgainstOther(startEntity, startEntityRow, volatileEndPosition, startEntitySide);
        trgPos = volatileEndPosition;
    }
    arrow->setPoints(QLineF(srcPos, trgPos), startEntitySide, endEntitySide);
}

QPointF ErdConnection::findThisPosAgainstOther(ErdEntity* thisEntity, int thisRow, const QPointF& otherPosition, ErdArrowItem::Side& entitySide)
{
    QPointF entityPos = thisEntity->pos();
    QRectF rowRect = (thisRow > -1) ? thisEntity->rowRect(thisRow) : QRectF(0, 0, 0, 0);
    QRectF entityRect = thisEntity->rect();
    entitySide = ErdArrowItem::LEFT;

    QPointF pos = entityPos + rowRect.topLeft() + QPointF(0, rowRect.height() / 2);
    if (otherPosition.x() > (entityPos.x() + (entityRect.width()) / 2))
    {
        pos.rx() += entityRect.width();
        entitySide = ErdArrowItem::RIGHT;
    }

    return pos;
}

ErdEntity* ErdConnection::getEndEntity() const
{
    return endEntity;
}

int ErdConnection::getStartEntityRow() const
{
    return startEntityRow;
}

int ErdConnection::getEndEntityRow() const
{
    return endEntityRow;
}

void ErdConnection::setArrowType(ErdArrowItem::Type arrowType)
{
    bool selectable = arrow->flags().testFlag(QGraphicsItem::ItemIsSelectable);
    scene->removeItem(arrow);
    delete arrow;

    arrow = ErdArrowItem::create(arrowType);
    arrow->setFlag(QGraphicsItem::ItemIsSelectable, selectable);
    scene->addItem(arrow);
    refreshPosition();
}

void ErdConnection::setIndexInStartEntity(int idx)
{
    arrow->setArrowIndexInStartEntity(idx);
}

void ErdConnection::setIndexInEndEntity(int idx)
{
    arrow->setArrowIndexInEndEntity(idx);
}

ErdEntity* ErdConnection::getStartEntity() const
{
    return startEntity;
}

