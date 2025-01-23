#include "erdview.h"
#include "erdconnection.h"
#include "erdentity.h"
#include "erdlinearrowitem.h"
#include "erdscene.h"
#include "common/unused.h"
#include "mainwindow.h"
#include "erdeditorplugin.h"
#include <QGraphicsItem>
#include <QMouseEvent>
#include <QDebug>
#include <QTimer>

ErdView::ErdView(QWidget* parent) :
    QGraphicsView(parent)
{
    setMouseTracking(true);
    setRenderHints(QPainter::Antialiasing);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    keyFilter = new KeyPressFilter(this);
    MAINWINDOW->installEventFilter(keyFilter);
}

ErdView::~ErdView()
{
    MAINWINDOW->removeEventFilter(keyFilter);
}

void ErdView::setScene(ErdScene* scene)
{
    QGraphicsView::setScene(scene);
    connect(scene, &ErdScene::showEntityToUser, this, &ErdView::showItemToUser);
}

ErdScene* ErdView::scene() const
{
    return qobject_cast<ErdScene*>(QGraphicsView::scene());
}

bool ErdView::isSpacePressed() const
{
    return spaceIsPressed;
}

void ErdView::mousePressEvent(QMouseEvent* event)
{
    if (spaceIsPressed)
    {
        QGraphicsView::mousePressEvent(event);
        return;
    }

    clickPos = event->pos();

    if (event->button() == Qt::LeftButton)
    {
        QGraphicsItem* item = clickableItemAt(clickPos);
        if (item && item->flags() & QGraphicsItem::ItemIsMovable)
        {
            if (selectedItems.contains(item))
            {
                for (QGraphicsItem* movableItem : selectedMovableItems)
                    dragOffset[movableItem] = transform().inverted().map(clickPos - mapFromScene(movableItem->pos()));

                return;
            }
            else
            {
                dragOffset.clear();
                dragOffset[item] = transform().inverted().map(clickPos - mapFromScene(item->pos()));
                if (draftingConnectionMode)
                    return;

                selectedItems = {item};
                selectedMovableItems = {item};
            }
        }
        else if (!spaceIsPressed)
        {
            clearSelectedItems();
            setDragMode(QGraphicsView::RubberBandDrag);
        }
    }
    else
    {
        clearSelectedItems();
    }

    QGraphicsView::mousePressEvent(event);
}

void ErdView::mouseMoveEvent(QMouseEvent* event)
{
    if (spaceIsPressed)
    {
        QGraphicsView::mouseMoveEvent(event);
        return;
    }

    if (!selectedItems.isEmpty() && event->buttons().testFlag(Qt::LeftButton))
    {
        for (QGraphicsItem* item : selectedMovableItems)
        {
            item->setPos(mapToScene(event->pos()) - dragOffset[item]);
            ErdEntity* entity = dynamic_cast<ErdEntity*>(item);
            if (entity)
                entity->updateConnectionsGeometry();
        }
        dynamic_cast<ErdScene*>(scene())->refreshSceneRect();
    }
    else if (draftConnection)
        draftConnection->updatePosition(mapToScene(event->pos()));

    clickPos = QPoint();

    QGraphicsView::mouseMoveEvent(event);
}

void ErdView::mouseReleaseEvent(QMouseEvent* event)
{
    if (spaceIsPressed)
    {
        QGraphicsView::mouseReleaseEvent(event);
        return;
    }

    dragOffset.clear();

    if (!clickPos.isNull() && event->pos() == clickPos)
    {
        clickPos = QPoint();
        if (viewClicked(event->pos(), event->button()))
            return;
    }

    setDragMode(QGraphicsView::NoDrag);
    handleSelectionOnMouseEvent(event->pos());

    QGraphicsView::mouseReleaseEvent(event);
}

void ErdView::mouseDoubleClickEvent(QMouseEvent* event)
{
    UNUSED(event);
    abortDraftConnection();
}

void ErdView::focusOutEvent(QFocusEvent* event)
{
    spaceReleased();
    QGraphicsView::focusOutEvent(event);
}

void ErdView::applyZoomRatio(qreal ratio)
{
    zoom *= ratio;
    if (zoom > 1.0) {
        resetZoom();
        return;
    }
    scale(ratio, ratio);
}

void ErdView::wheelEvent(QWheelEvent* event)
{
    qreal diff = ((qreal)event->angleDelta().y()) / 1500;
    qreal ratio = diff >= 0 ? (1.0 + diff) : (1 / (1.0 - diff));
    applyZoomRatio(ratio);
}

bool ErdView::event(QEvent* event)
{
    if (event->type() == QEvent::Resize)
    {
        bool res = QGraphicsView::event(event);
        if (!centerPointRestored && !centerPoint.isNull())
            centerOn(centerPoint);

        return res;
    }
    return QGraphicsView::event(event);
}

qreal ErdView::getZoom() const
{
    return zoom;
}

void ErdView::applyConfig(const QHash<QString, QVariant> &erdConfig)
{
    QVariant cfgZoom = erdConfig[CFG_KEY_ZOOM];
    if (!cfgZoom.isNull())
    {
        qreal zoom = cfgZoom.toReal();
        if ((1.0 - zoom) > 0.0001 && zoom > 0.01) // excluding any floating-point micro-differences and excluding too far zoom out
            applyZoomRatio(zoom);
    }

    centerPoint = erdConfig[CFG_KEY_CENTER_POINT].toPointF();
    if (!centerPoint.isNull())
    {
        centerOn(centerPoint);
        QTimer::singleShot(0, [this]() {
            centerPointRestored = true;
        });
    }
}

QHash<QString, QVariant> ErdView::getConfig()
{
    QHash<QString, QVariant> erdConfig;
    erdConfig[CFG_KEY_CENTER_POINT] = mapToScene(viewport()->rect().center());
    erdConfig[CFG_KEY_ZOOM] = getZoom();
    return erdConfig;
}

bool ErdView::viewClicked(const QPoint& pos, Qt::MouseButton button)
{
    if (button == Qt::LeftButton)
    {
        QGraphicsItem* item = clickableItemAt(pos);
        ErdEntity* entity = dynamic_cast<ErdEntity*>(item);
        if (entity)
        {
            int rowIdx = entity->rowIndexAt(mapToScene(pos));
            if (rowIdx <= 0)
                return false;

            if (draftConnection)
            {
                ErdEntity* entity = dynamic_cast<ErdEntity*>(item);
                draftConnection->finalizeConnection(entity, mapToScene(pos));
                draftConnection = nullptr;
                draftingConnectionMode = false;
                emit draftConnectionRemoved();
            }
            else if (draftingConnectionMode)
            {
                ErdEntity* entity = dynamic_cast<ErdEntity*>(item);
                draftConnection = new ErdConnection(entity, mapToScene(pos), scene()->getArrowType());
                draftConnection->addToScene(scene());
                draftConnection->select();
            }
            return true;
        }
        return false;
    }
    else if (button == Qt::RightButton)
    {
        if (draftConnection)
        {
            abortDraftConnection();
        }
        else
        {
            // TODO if clicked on item, show entity/connection/other context menu
            // TODO if clicked on empty space, show creational menu, having "add this and that, rearrange, select all" entries
        }
    }
    return false;
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
    for (QGraphicsItem* item : scene()->items())
        item->setAcceptedMouseButtons(Qt::NoButton);
}

void ErdView::spaceReleased()
{
    spaceIsPressed = false;
    setDragMode(QGraphicsView::NoDrag);

    for (QGraphicsItem* item : scene()->items())
        item->setAcceptedMouseButtons(Qt::AllButtons);
}

void ErdView::resetZoom()
{
    resetTransform();
    zoom = 1.0;
}

void ErdView::showItemToUser(QGraphicsItem* item)
{
    centerOn(item);
}

void ErdView::abortDraftConnection()
{
    safe_delete(draftConnection);
    emit draftConnectionRemoved();
}

void ErdView::setDraftingConnectionMode(bool enabled)
{
    draftingConnectionMode = enabled;
    if (!enabled)
        abortDraftConnection();
}

void ErdView::handleSelectionOnMouseEvent(const QPoint& pos)
{
    QGraphicsItem* item = clickableItemAt(pos);
    if (selectedItems.contains(item))
        return; // button relesed over selected item - no deselection

    selectedItems = scene()->selectedItems();
    selectedMovableItems = filter<QGraphicsItem*>(selectedItems, [](QGraphicsItem* item) -> bool
    {
       return item->flags().testFlag(QGraphicsItem::ItemIsMovable);
    });
}

void ErdView::clearSelectedItems()
{
    selectedItems.clear();
    selectedMovableItems.clear();
    dragOffset.clear();
}

ErdView::KeyPressFilter::KeyPressFilter(ErdView* view) :
    QObject(view), view(view)
{
}

bool ErdView::KeyPressFilter::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->isAutoRepeat())
            return QObject::eventFilter(obj, event);

        if (keyEvent->key() == Qt::Key_Space) {
            view->spacePressed();
        }
    } else if (event->type() == QEvent::KeyRelease) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->isAutoRepeat())
            return QObject::eventFilter(obj, event);

        if (keyEvent->key() == Qt::Key_Space) {
            view->spaceReleased();
        }
    }
    return QObject::eventFilter(obj, event);
}
