#include "erdview.h"
#include "erdconnection.h"
#include "erdentity.h"
#include "erdlinearrowitem.h"
#include "erdscene.h"
#include "common/unused.h"
#include "mainwindow.h"
#include "erdeditorplugin.h"
#include "db/db.h"
#include "erdwindow.h"
#include "icon.h"
#include <QGraphicsItem>
#include <QMouseEvent>
#include <QDebug>
#include <QTimer>
#include <QWindow>

ErdView::ErdView(QWidget* parent) :
    QGraphicsView(parent)
{
    setMouseTracking(true);
    setRenderHints(QPainter::Antialiasing);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    keyFilter = new KeyPressFilter(this);
    MAINWINDOW->installEventFilter(keyFilter);

    new QShortcut(QKeySequence::Delete, this, SLOT(deleteSelectedItem()), SLOT(deleteSelectedItem()), Qt::WidgetWithChildrenShortcut);
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

Db *ErdView::getDb() const
{
    if (!QGraphicsView::scene())
        return nullptr;

    return scene()->getDb();
}

void ErdView::mousePressEvent(QMouseEvent* event)
{
    if (isDragging())
    {
        QGraphicsView::mousePressEvent(event);
        return;
    }

    clickPos = event->position().toPoint();

    if (isPlacingNewEntity())
    {
        if (event->button() == Qt::LeftButton)
        {
            // All good. Proceed to further events for complete click.
            return;
        }
        else
            popOperatingMode();
    }

    if (event->button() == Qt::LeftButton)
    {
        QGraphicsItem* item = clickableItemAt(clickPos);
        if (item && item->flags() & QGraphicsItem::ItemIsMovable)
        {
            if (selectedItems.contains(item))
            {
                for (QGraphicsItem* movableItem : selectedMovableItems)
                    dragOffset[movableItem] = transform().inverted().map(clickPos - mapFromScene(movableItem->pos()));

                QGraphicsView::mousePressEvent(event);
                return;
            }
            else
            {
                dragOffset.clear();
                dragOffset[item] = transform().inverted().map(clickPos - mapFromScene(item->pos()));
                if (isDraftingConnection())
                    return;

                selectedItems = {item};
                selectedMovableItems = {item};
            }
        }
        else if (!isDragging())
        {
            clearSelectedItems();
            setDragMode(QGraphicsView::RubberBandDrag);
        }
    }
    else if (event->button() == Qt::MiddleButton)
    {
        clearSelectedItems();
    }
    else
    {
        // RightButton click
        if (isDraftingConnection() || isPlacingNewEntity())
            popOperatingMode();
    }

    QGraphicsView::mousePressEvent(event);
}

void ErdView::mouseMoveEvent(QMouseEvent* event)
{
    if (isDragging())
    {
        QGraphicsView::mouseMoveEvent(event);
        return;
    }

    if (isPlacingNewEntity())
        return;

    if (!selectedItems.isEmpty() && event->buttons().testFlag(Qt::LeftButton))
    {
        for (QGraphicsItem* item : selectedMovableItems)
        {
            item->setPos(mapToScene(event->position().toPoint()) - dragOffset[item]);
            ErdEntity* entity = dynamic_cast<ErdEntity*>(item);
            if (entity)
                entity->updateConnectionsGeometry();
        }
        dynamic_cast<ErdScene*>(scene())->refreshSceneRect();
    }
    else if (draftConnection)
        draftConnection->updatePosition(mapToScene(event->position().toPoint()));

    clickPos = QPoint();

    QGraphicsView::mouseMoveEvent(event);
}

void ErdView::mouseReleaseEvent(QMouseEvent* event)
{
    if (isDragging())
    {
        QGraphicsView::mouseReleaseEvent(event);
        return;
    }

    dragOffset.clear();

    if (isPlacingNewEntity())
    {
        popOperatingMode();
        if (event->button() == Qt::LeftButton)
        {
            emit newEntityPositionPicked(mapToScene(event->pos()));
            return;
        }
    }

    if (!clickPos.isNull() && event->position().toPoint() == clickPos)
    {
        clickPos = QPoint();
        if (viewClicked(event->position().toPoint(), event->button()))
            return;
    }

    setDragMode(QGraphicsView::NoDrag);
    handleSelectionOnMouseEvent(event->position().toPoint());

    QGraphicsView::mouseReleaseEvent(event);
}

bool ErdView::viewClicked(const QPoint& pos, Qt::MouseButton button)
{
    if (button == Qt::LeftButton)
    {
        handleConnectionClick(pos);
    }
    if (button == Qt::MiddleButton)
    {
        handleConnectionClick(pos, true);
    }
    else if (button == Qt::RightButton)
    {
        // TODO if clicked on item, show entity/connection/other context menu
        // TODO if clicked on empty space, show creational menu, having "add this and that, rearrange, select all" entries
        return true;
    }
    return false;
}

void ErdView::mouseDoubleClickEvent(QMouseEvent* event)
{
    UNUSED(event);
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
        QTimer::singleShot(1000, [this]() {
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

bool ErdView::handleConnectionClick(const QPoint& pos, bool enableConnectionDrafting)
{
    QGraphicsItem* item = clickableItemAt(pos);
    ErdEntity* entity = dynamic_cast<ErdEntity*>(item);
    if (entity)
    {
        int rowIdx = entity->rowIndexAt(mapToScene(pos));
        if (rowIdx <= 0)
            return false;

        if (enableConnectionDrafting && !isDraftingConnection())
            setOperatingMode(Mode::CONNECTION_DRAFTING);

        if (draftConnection)
        {
            draftConnection->finalizeConnection(entity, mapToScene(pos));
            draftConnection = nullptr;

            setOperatingMode(Mode::NORMAL);
            return true;
        }
        else if (isDraftingConnection())
        {
            draftConnection = new ErdConnection(entity, mapToScene(pos), scene()->getArrowType());
            draftConnection->addToScene(scene());
            return true;
        }
    }
    return false;
}

void ErdView::spacePressed()
{
    pushOperatingMode(Mode::DRAGGING);
}

void ErdView::spaceReleased()
{
    if (isDragging())
        popOperatingMode();
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

void ErdView::deleteSelectedItem()
{
    scene()->deleteItems(selectedItems);
}

void ErdView::leavingOperatingMode(Mode mode)
{
    switch (mode)
    {
        case ErdView::Mode::NORMAL:
            break;
        case ErdView::Mode::DRAGGING:
            setDragMode(QGraphicsView::NoDrag);
            for (QGraphicsItem* item : scene()->items())
                item->setAcceptedMouseButtons(Qt::AllButtons);
            break;
        case ErdView::Mode::CONNECTION_DRAFTING:
            applyCursor(nullptr);
            abortDraftConnection();
            break;
        case ErdView::Mode::PLACING_NEW_ENTITY:
            applyCursor(nullptr);
            emit tableInsertionAborted();
            break;
    }
}

void ErdView::enteringOperatingMode(Mode mode)
{
    switch (mode)
    {
        case ErdView::Mode::NORMAL:
            break;
        case ErdView::Mode::DRAGGING:
            setDragMode(QGraphicsView::ScrollHandDrag);
            for (QGraphicsItem* item : scene()->items())
                item->setAcceptedMouseButtons(Qt::NoButton);
            break;
        case ErdView::Mode::CONNECTION_DRAFTING:
            applyCursor(ErdWindow::cursorFkIcon->toQIconPtr());
            break;
        case ErdView::Mode::PLACING_NEW_ENTITY:
            applyCursor(ErdWindow::cursorAddTableIcon->toQIconPtr());
            break;
    }
}

void ErdView::abortDraftConnection()
{
    safe_delete(draftConnection);
    emit draftConnectionRemoved();
}

void ErdView::setDraftingConnectionMode(bool enabled)
{
    if (enabled)
        setOperatingMode(Mode::CONNECTION_DRAFTING);
    else
        setOperatingMode(Mode::NORMAL);
}

void ErdView::insertNewEntity()
{
    setOperatingMode(Mode::PLACING_NEW_ENTITY);
}

void ErdView::handleSelectionOnMouseEvent(const QPoint& pos)
{
    QGraphicsItem* item = clickableItemAt(pos);
    if (selectedItems.contains(item))
        return; // button relesed over selected item - no deselection

    selectedItems = scene()->selectedItems();
    selectedMovableItems = selectedItems | FILTER(item, {return item->flags().testFlag(QGraphicsItem::ItemIsMovable);});
}

void ErdView::clearSelectedItems()
{
    selectedItems.clear();
    selectedMovableItems.clear();
    dragOffset.clear();
    scene()->clearSelection();
    scene()->clearFocus();
}

bool ErdView::isDragging() const
{
    return operatingMode == Mode::DRAGGING;
}

bool ErdView::isDraftingConnection() const
{
    return operatingMode == Mode::CONNECTION_DRAFTING;
}

bool ErdView::isPlacingNewEntity() const
{
    return operatingMode == Mode::PLACING_NEW_ENTITY;
}

void ErdView::setOperatingMode(Mode mode)
{
    priorOperatingModeStack.clear();

    if (operatingMode == mode)
        return;

    leavingOperatingMode(operatingMode);
    operatingMode = mode;
    enteringOperatingMode(mode);
}

void ErdView::pushOperatingMode(Mode mode)
{
    priorOperatingModeStack.push(operatingMode);
    operatingMode = mode;
    enteringOperatingMode(mode);
}

void ErdView::popOperatingMode()
{
    if (priorOperatingModeStack.isEmpty())
        setOperatingMode(Mode::NORMAL);
    else
    {
        leavingOperatingMode(operatingMode);
        operatingMode = priorOperatingModeStack.pop();
    }
}

void ErdView::applyCursor(QIcon* icon)
{
    if (!icon)
    {
        setCursor(QCursor());
        return;
    }

    QWindow *w = window()->windowHandle();
    qreal dpr = w ? w->devicePixelRatio() : qApp->devicePixelRatio();

    const int logicalSize = 32;
    QSize logical(logicalSize, logicalSize);
    QSize physical = logical * dpr;

    QPixmap pm = icon->pixmap(physical);
    pm.setDevicePixelRatio(dpr);

    QCursor cursor(pm, 1 * dpr, 1 * dpr);
    setCursor(cursor);
}

ErdView::KeyPressFilter::KeyPressFilter(ErdView* view) :
    QObject(view), view(view)
{
}

bool ErdView::KeyPressFilter::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->isAutoRepeat())
            return QObject::eventFilter(obj, event);

        if (keyEvent->key() == Qt::Key_Space)
            view->spacePressed();
    }
    else if (event->type() == QEvent::KeyRelease)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->isAutoRepeat())
            return QObject::eventFilter(obj, event);

        if (keyEvent->key() == Qt::Key_Space)
            view->spaceReleased();
    }
    return QObject::eventFilter(obj, event);
}

QDebug operator<<(QDebug dbg, ErdView::Mode value)
{
    QDebugStateSaver saver(dbg);
    switch (value)
    {
        case ErdView::Mode::NORMAL:
            dbg << "NORMAL";
            break;
        case ErdView::Mode::DRAGGING:
            dbg << "DRAGGING";
            break;
        case ErdView::Mode::CONNECTION_DRAFTING:
            dbg << "CONNECTION_DRAFTING";
            break;
        case ErdView::Mode::PLACING_NEW_ENTITY:
            dbg << "PLACING_NEW_ENTITY";
            break;
    }
    return dbg;
}
