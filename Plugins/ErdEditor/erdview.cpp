#include "erdview.h"
#include "erdconnection.h"
#include "erdentity.h"
#include "erdlinearrowitem.h"
#include "erdscene.h"
#include "common/unused.h"
#include <QGraphicsItem>
#include <QMouseEvent>
#include <QDebug>

ErdView::ErdView(QWidget* parent) :
    QGraphicsView(parent)
{
    setMouseTracking(true);
    setRenderHints(QPainter::Antialiasing);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
}

void ErdView::mousePressEvent(QMouseEvent* event)
{
    if (spaceIsPressed)
    {
        QGraphicsView::mousePressEvent(event);
        return;
    }

    clickPos = event->pos();

    QGraphicsItem* item = clickableItemAt(event->pos());
    if (item && item->flags() & QGraphicsItem::ItemIsSelectable)
    {
        selectedItem = item;
        dragOffset = transform().inverted().map(event->pos() - mapFromScene(item->pos()));
    }
    else if (!spaceIsPressed)
        setDragMode(QGraphicsView::RubberBandDrag);

    QGraphicsView::mousePressEvent(event);
}

void ErdView::mouseMoveEvent(QMouseEvent* event)
{
    if (selectedItem)
    {
        selectedItem->setPos(mapToScene(event->pos()) - dragOffset);
        ErdEntity* entity = dynamic_cast<ErdEntity*>(selectedItem);
        if (entity)
            entity->updateConnectionsGeometry();

        dynamic_cast<ErdScene*>(scene())->refreshSceneRect();
    }
    else if (currentConnection)
        currentConnection->updatePosition(mapToScene(event->pos()));

    clickPos = QPoint();

    QGraphicsView::mouseMoveEvent(event);
}

void ErdView::mouseReleaseEvent(QMouseEvent* event)
{
    selectedItem = nullptr;

    if (!clickPos.isNull() && event->pos() == clickPos)
    {
        clickPos = QPoint();
        viewClicked(event->pos());
    }

    if (!spaceIsPressed)
        setDragMode(QGraphicsView::NoDrag);

    QGraphicsView::mouseReleaseEvent(event);
}

void ErdView::mouseDoubleClickEvent(QMouseEvent* event)
{
    UNUSED(event);
    if (currentConnection)
    {
        delete currentConnection;
        currentConnection = nullptr;
    }
}

void ErdView::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Space)
    {
        if (event->isAutoRepeat())
            return;

        spacePressed();
    }

    QGraphicsView::keyPressEvent(event);
}

void ErdView::keyReleaseEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Space)
    {
        if (event->isAutoRepeat())
            return;

        spaceReleased();
    }

    QGraphicsView::keyReleaseEvent(event);
}

void ErdView::focusOutEvent(QFocusEvent* event)
{
    spaceReleased();
    QGraphicsView::focusOutEvent(event);
}

void ErdView::wheelEvent(QWheelEvent* event)
{
    qreal diff = ((qreal)event->angleDelta().y()) / 300;
    qreal ratio = diff >= 0 ? (1.0 + diff) : (1 / (1.0 - diff));
    zoom *= ratio;
    if (zoom > 1.0) {
        resetZoom();
        return;
    }
    scale(ratio, ratio);
}

void ErdView::viewClicked(const QPoint& pos)
{
    QGraphicsItem* item = clickableItemAt(pos);
    ErdEntity* entity = dynamic_cast<ErdEntity*>(item);
    if (entity)
    {
        int rowIdx = entity->rowIndexAt(mapToScene(pos));
        if (rowIdx <= 0)
            return;

        if (currentConnection)
        {
            ErdEntity* entity = dynamic_cast<ErdEntity*>(item);
            currentConnection->finalizeConnection(entity, mapToScene(pos));
            currentConnection = nullptr;
        }
        else
        {
            ErdEntity* entity = dynamic_cast<ErdEntity*>(item);
            currentConnection = new ErdConnection(entity, mapToScene(pos));
            currentConnection->addToScene(scene());
        }
    }
}

QGraphicsItem* ErdView::clickableItemAt(const QPoint& pos)
{
    QList<QGraphicsItem*> list = items(pos);
    for (auto it = list.begin(); it != list.end(); it++)
    {
        ErdItem* erdItem = dynamic_cast<ErdItem*>(*it);
        if (erdItem && erdItem->isClickable())
            return *it;
    }
    return nullptr;
}

void ErdView::spacePressed()
{
    spaceIsPressed = true;
    setDragMode(QGraphicsView::ScrollHandDrag);
}

void ErdView::spaceReleased()
{
    spaceIsPressed = false;
    setDragMode(QGraphicsView::NoDrag);
}

void ErdView::resetZoom()
{
    resetTransform();
    zoom = 1.0;
}
