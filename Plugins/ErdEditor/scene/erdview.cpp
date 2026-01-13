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
#include "changes/erdchangemoveentity.h"
#include "services/notifymanager.h"
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
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
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

Db *ErdView::getDb() const
{
    if (!QGraphicsView::scene())
        return nullptr;

    return scene()->getDb();
}

void ErdView::mousePressEvent(QMouseEvent* event)
{
    if (isDraggingView())
    {
        QGraphicsView::mousePressEvent(event);
        return;
    }

    clickPos = event->position().toPoint();
    lastClickPos = mapToScene(event->position().toPoint());

    if (isPlacingNewEntity())
    {
        if (event->button() == Qt::LeftButton)
        {
            // All good. Proceed to further events for complete click.
            return;
        }
        else
        {
            popOperatingMode();
            return;
        }
    }

    QGraphicsItem* item = clickableItemAt(clickPos);
    if (event->button() == Qt::LeftButton || event->button() == Qt::RightButton)
    {
        if (item && item->flags() & QGraphicsItem::ItemIsMovable)
        {
            if (selectedItems.contains(item))
            {
                for (QGraphicsItem* movableItem : selectedMovableItems)
                {
                    dragOffset[movableItem] = transform().inverted().map(clickPos - mapFromScene(movableItem->pos()));
                    dragStartPos[movableItem] = movableItem->pos();
                }
            }
            else
            {
                dragOffset.clear();
                dragOffset[item] = transform().inverted().map(clickPos - mapFromScene(item->pos()));
                dragStartPos[item] = item->pos();
                if (isDraftingConnection())
                    return;

                selectedItems = {item};
                selectedMovableItems = {item};
            }
        }
    }

    if (event->button() == Qt::LeftButton)
    {
        if (item && item->flags() & QGraphicsItem::ItemIsMovable)
        {
            lastDragScenePos = mapToScene(clickPos);
            QGraphicsView::mousePressEvent(event);
            return;
        }

        if (!isDraggingView())
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
        {
            popOperatingMode();
        }
        else
        {
            // Fake left-click to force selection
            QMouseEvent leftBtnEvent(event->type(), event->position(), event->globalPosition(), Qt::LeftButton,
                Qt::LeftButton | (event->buttons() & ~Qt::RightButton), event->modifiers());

            QGraphicsView::mousePressEvent(&leftBtnEvent);
            event->ignore();

            emit customContextMenuRequested(event->pos());
        }
    }
    QGraphicsView::mousePressEvent(event);
}

void ErdView::mouseMoveEvent(QMouseEvent* event)
{
    if (isDraggingView())
    {
        QGraphicsView::mouseMoveEvent(event);
        return;
    }

    if (isPlacingNewEntity())
        return;

    if (!selectedItems.isEmpty() && event->buttons().testFlag(Qt::LeftButton) && !dragOffset.isEmpty())
    {
        // The code below sets position for same items twice: before and after scene rect is updated.
        // The scene rect is resized to reflect currently needed area - it's just nice for UX.
        // Unfortunately updating scene rect messes up positions updated for items just a moment before,
        // probably because moved items are still in some kind of "dirty" state according to the scene,
        // so updating scene rect messes up their positions.
        // Setting same positions afterwards fixes the issue of flickering during items D&D.

        // Set new positions initially
        QPointF scenePos = mapToScene(event->position().toPoint());
        for (QGraphicsItem* item : selectedMovableItems)
            item->setPos(scenePos - dragOffset[item]);

        // Update scene rect to expand/shrink it
        dynamic_cast<ErdScene*>(scene())->refreshSceneRect();

        // Set new positions once again, cause new scene rect could break positioning
        for (QGraphicsItem* item : selectedMovableItems)
            item->setPos(scenePos - dragOffset[item]);

        // Update connection arrows
        for (QGraphicsItem* item : selectedMovableItems)
        {
            ErdEntity* entity = dynamic_cast<ErdEntity*>(item);
            if (entity)
                entity->updateConnectionsGeometry();
        }
        return;
    }
    else if (draftConnection)
        draftConnection->updatePosition(mapToScene(event->position().toPoint()));

    if (!tolerateMicroMovesForClick())
        clickPos = QPoint();

    QGraphicsView::mouseMoveEvent(event);
}

void ErdView::mouseReleaseEvent(QMouseEvent* event)
{
    if (isDraggingView())
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

    if (event->button() == Qt::LeftButton)
        itemsPotentiallyMoved();

    if (!clickPos.isNull() && (event->position().toPoint() == clickPos || sameItemOnPositions(clickPos, event->position().toPoint())))
    {
        QPoint pressPos = clickPos;
        clickPos = QPoint();
        if (viewClicked(pressPos, event->button()))
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
        return true;
    }
    return false;
}

void ErdView::mouseDoubleClickEvent(QMouseEvent* event)
{
    dragOffset.clear();
    if (event->button() == Qt::LeftButton)
    {
        QGraphicsItem* item = clickableItemAt(event->pos());
        ErdEntity* entity = dynamic_cast<ErdEntity*>(item);
        if (entity)
        {
            entity->edit(mapToScene(event->pos()));
            return;
        }
        ErdArrowItem* arrow = dynamic_cast<ErdArrowItem*>(item);
        if (arrow)
        {
            ErdConnection* conn = scene()->getConnectionForArrow(arrow);
            if (!conn || !conn->isFinalized())
                return;

            if (conn->isCompoundConnection())
            {
                notifyInfo(tr("Cannot edit compound foreign keys this way. Such connections have to be edited through the side panel.", "ERD editor"));
                return;
            }

            conn->cancelFinalState(mapToScene(event->pos()));
            draftConnection = conn;
            setOperatingMode(Mode::CONNECTION_DRAFTING);

            return;
        }
    }
}

void ErdView::focusOutEvent(QFocusEvent* event)
{
    spaceReleased();
    QGraphicsView::focusOutEvent(event);
}

void ErdView::applyZoomRatio(qreal ratio)
{
    zoom *= ratio;
    if (zoom > 1.0)
    {
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
        resetZoom();
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
    pushOperatingMode(Mode::DRAGGING_VIEW);
}

void ErdView::spaceReleased()
{
    if (isDraggingView())
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

void ErdView::handleVisibilityRequest(const QRectF& rect)
{
    ensureVisible(rect);
}

void ErdView::leavingOperatingMode(Mode mode)
{
    switch (mode)
    {
        case ErdView::Mode::NORMAL:
            break;
        case ErdView::Mode::DRAGGING_VIEW:
            endDragBySpace();
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
        case ErdView::Mode::DRAGGING_VIEW:
            setDragMode(QGraphicsView::ScrollHandDrag);
            for (QGraphicsItem* item : scene()->items())
                item->setAcceptedMouseButtons(Qt::NoButton);
            startDragBySpace();
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
    if (draftConnection && draftConnection->isEditing())
    {
        draftConnection->restoreFinalState();
        draftConnection = nullptr;
        return;
    }

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

bool ErdView::isDraggingView() const
{
    return operatingMode == Mode::DRAGGING_VIEW;
}

bool ErdView::isDraftingConnection() const
{
    return operatingMode == Mode::CONNECTION_DRAFTING;
}

bool ErdView::isPlacingNewEntity() const
{
    return operatingMode == Mode::PLACING_NEW_ENTITY;
}

bool ErdView::isNormalMode() const
{
    return operatingMode == Mode::NORMAL;
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
        viewport()->unsetCursor();
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
    viewport()->setCursor(cursor);
}

bool ErdView::sameItemOnPositions(const QPoint& pos1, const QPoint& pos2)
{
    QList<QGraphicsItem*> list1 = items(pos1);
    QList<QGraphicsItem*> list2 = items(pos2);
    for (QGraphicsItem*& it1 : list1)
        if (list2.contains(it1))
            return true;

    return false;
}

bool ErdView::tolerateMicroMovesForClick()
{
    // Even if not started the arrow yet, we want initial click to be tolerant for micro-moves
    if (isDraftingConnection())
        return true;

    return false;
}

void ErdView::startDragBySpace()
{
    if (CFG_ERD.Erd.DragBySpace.get())
    {
        QPoint globalPos = QCursor::pos();
        QPoint pos = viewport()->mapFromGlobal(globalPos);
        QMouseEvent press(QEvent::MouseButtonPress, pos, globalPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        QCoreApplication::sendEvent(viewport(), &press);
    }
}

void ErdView::endDragBySpace()
{
    if (CFG_ERD.Erd.DragBySpace.get())
    {
        QPoint globalPos = QCursor::pos();
        QPoint pos = viewport()->mapFromGlobal(globalPos);
        QMouseEvent release(QEvent::MouseButtonRelease, pos, globalPos, Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
        QCoreApplication::sendEvent(viewport(), &release);
    }
}

void ErdView::itemsPotentiallyMoved()
{
    QList<ErdChange*> changes;
    QStringList names;
    for (QGraphicsItem* item : selectedMovableItems)
    {
        ErdEntity* entity = dynamic_cast<ErdEntity*>(item);
        if (entity)
        {
            if (dragStartPos[item] != item->pos())
            {
                changes << new ErdChangeMoveEntity(entity->getTableName(), dragStartPos[item], item->pos(),
                                                    tr("Move table \"%1\"").arg(entity->getTableName()));
            }
            names << entity->getTableName();
        }
    }

    ErdChange* change = ErdChange::normalizeChanges(changes, tr("Move tables: %1").arg(names.join(", ")));
    if (change)
        emit changeCreated(change);
}

QPointF ErdView::getLastClickPos() const
{
    return lastClickPos;
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
        case ErdView::Mode::DRAGGING_VIEW:
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
