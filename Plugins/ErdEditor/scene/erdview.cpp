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
    switch (operatingMode)
    {
        case ErdView::Mode::NORMAL:
            mousePressedNormal(event);
            break;
        case ErdView::Mode::DRAGGING_VIEW:
            mousePressedDraggingView(event);
            break;
        case ErdView::Mode::CONNECTION_DRAFTING:
            mousePressedConnectionDrafting(event);
            break;
        case ErdView::Mode::PLACING_NEW_ENTITY:
            mousePressedPlacingNewEntity(event);
            break;
        case ErdView::Mode::AREA_SELECTING:
            mousePressedAreaSelecting(event);
            break;
    }
}

void ErdView::mouseMoveEvent(QMouseEvent* event)
{
    switch (operatingMode)
    {
        case ErdView::Mode::NORMAL:
            mouseMovedNormal(event);
            break;
        case ErdView::Mode::DRAGGING_VIEW:
            mouseMovedDraggingView(event);
            break;
        case ErdView::Mode::CONNECTION_DRAFTING:
            mouseMovedConnectionDrafting(event);
            break;
        case ErdView::Mode::PLACING_NEW_ENTITY:
            mouseMovedPlacingNewEntity(event);
            break;
        case ErdView::Mode::AREA_SELECTING:
            mouseMovedAreaSelecting(event);
            break;
    }
}

void ErdView::mouseReleaseEvent(QMouseEvent* event)
{
    switch (operatingMode)
    {
        case ErdView::Mode::NORMAL:
            mouseReleasedNormal(event);
            break;
        case ErdView::Mode::DRAGGING_VIEW:
            mouseReleasedDraggingView(event);
            break;
        case ErdView::Mode::CONNECTION_DRAFTING:
            mouseReleasedConnectionDrafting(event);
            break;
        case ErdView::Mode::PLACING_NEW_ENTITY:
            mouseReleasedPlacingNewEntity(event);
            break;
        case ErdView::Mode::AREA_SELECTING:
            mouseReleasedAreaSelecting(event);
            break;
    }
}

void ErdView::mousePressedNormal(QMouseEvent* event)
{
    lastClickPos = mapToScene(event->position().toPoint()); // for context menu

    if (event->button() == Qt::RightButton)
    {
        // Fake left-click to force selection
        QMouseEvent leftBtnEvent(event->type(), event->position(), event->globalPosition(), Qt::LeftButton,
            Qt::LeftButton | (event->buttons() & ~Qt::RightButton), event->modifiers());

        QGraphicsView::mousePressEvent(&leftBtnEvent);
        event->ignore();

        emit customContextMenuRequested(event->pos());
        return;
    }

    QGraphicsItem* item = clickableItemAt(event->position().toPoint());

    // Area selection
    if (!item || !item->flags().testFlag(QGraphicsItem::ItemIsMovable))
    {
        if (!event->modifiers().testFlag(Qt::ControlModifier))
            clearSelection();

        pushOperatingMode(Mode::AREA_SELECTING);
        QGraphicsView::mousePressEvent(event);
        return;
    }

    // Enforced connection creation
    if (event->button() == Qt::MiddleButton)
    {
        setOperatingMode(Mode::CONNECTION_DRAFTING);
        mousePressedConnectionDrafting(event);
        return;
    }

    // Create similar entity by alt+drag
    ErdEntity* entity = dynamic_cast<ErdEntity*>(item);
    if (event->button() == Qt::LeftButton && event->modifiers().testFlag(Qt::AltModifier) && entity &&
        erdWindow->clearSidePanel())
    {
        clearSelection();
        createSimilar(entity);
        QGraphicsView::mousePressEvent(event);
        return;
    }

    QGraphicsView::mousePressEvent(event);

    // When starting to drag selected item(s)
    for (QGraphicsItem* it : scene()->selectedItems())
    {
        if (it->flags().testFlag(QGraphicsItem::ItemIsMovable))
            dragStartPos[it] = it->pos();
    }
    if (item && item->flags().testFlag(QGraphicsItem::ItemIsMovable)) // this item is not selected yet in the press event
        dragStartPos[item] = item->pos();
}

void ErdView::mousePressedDraggingView(QMouseEvent* event)
{
    QGraphicsView::mousePressEvent(event);
}

void ErdView::mousePressedConnectionDrafting(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton)
    {
        popOperatingMode();
        return;
    }

    QPoint pos = event->position().toPoint();
    QGraphicsItem* item = clickableItemAt(pos);
    ErdEntity* entity = dynamic_cast<ErdEntity*>(item);
    if (entity)
    {
        int rowIdx = entity->rowIndexAt(mapToScene(pos));
        if (rowIdx <= 0)
            return; // Cannot create connection to entity name or invalid area

        if (draftConnection)
        {
            draftConnection->finalizeConnection(entity, mapToScene(pos));
            draftConnection = nullptr;
            setOperatingMode(Mode::NORMAL);
        }
        else
        {
            draftConnection = new ErdConnection(entity, mapToScene(pos), scene()->getArrowType());
            draftConnection->addToScene(scene());
        }
        return;
    }

    QGraphicsView::mousePressEvent(event);
}

void ErdView::mousePressedPlacingNewEntity(QMouseEvent* event)
{
    if (!event->modifiers().testFlag(Qt::ShiftModifier))
        popOperatingMode();

    if (event->button() == Qt::LeftButton)
    {
        emit newEntityPositionPicked(mapToScene(event->pos()));
        return;
    }

    QGraphicsView::mousePressEvent(event);
}

void ErdView::mousePressedAreaSelecting(QMouseEvent* event)
{
    QGraphicsView::mousePressEvent(event);
}

void ErdView::mouseMovedNormal(QMouseEvent* event)
{
    QGraphicsView::mouseMoveEvent(event);
}

void ErdView::mouseMovedDraggingView(QMouseEvent* event)
{
    QGraphicsView::mouseMoveEvent(event);
}

void ErdView::mouseMovedConnectionDrafting(QMouseEvent* event)
{
    if (draftConnection)
        draftConnection->updatePosition(mapToScene(event->position().toPoint()));

    QGraphicsView::mouseMoveEvent(event);
}

void ErdView::mouseMovedPlacingNewEntity(QMouseEvent* event)
{
    QGraphicsView::mouseMoveEvent(event);
}

void ErdView::mouseMovedAreaSelecting(QMouseEvent* event)
{
    QGraphicsView::mouseMoveEvent(event);
}

void ErdView::mouseReleasedNormal(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
        itemsPotentiallyMoved();

    dragStartPos.clear();
    QGraphicsView::mouseReleaseEvent(event);
}

void ErdView::mouseReleasedDraggingView(QMouseEvent* event)
{
    QGraphicsView::mouseReleaseEvent(event);
}

void ErdView::mouseReleasedConnectionDrafting(QMouseEvent* event)
{
    QGraphicsView::mouseReleaseEvent(event);
}

void ErdView::mouseReleasedPlacingNewEntity(QMouseEvent* event)
{
    QGraphicsView::mouseReleaseEvent(event);
}

void ErdView::mouseReleasedAreaSelecting(QMouseEvent* event)
{
    QGraphicsView::mouseReleaseEvent(event);
    popOperatingMode();
}

void ErdView::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        QGraphicsItem* item = clickableItemAt(event->pos());
        ErdEntity* entity = dynamic_cast<ErdEntity*>(item);
        if (entity && event->modifiers().testFlag(Qt::ShiftModifier))
        {
            scene()->editEntityColumn(entity, mapToScene(event->pos()));
            return;
        }
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
    scene()->deleteItems(scene()->selectedItems());
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
        case ErdView::Mode::AREA_SELECTING:
            setDragMode(QGraphicsView::NoDrag);
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
        case ErdView::Mode::AREA_SELECTING:
            setDragMode(QGraphicsView::RubberBandDrag);
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
    for (QGraphicsItem* item : scene()->selectedItems())
    {
        ErdEntity* entity = dynamic_cast<ErdEntity*>(item);
        if (entity)
        {
            if (dragStartPos.contains(item) && dragStartPos[item] != item->pos())
            {
                changes << new ErdChangeMoveEntity(entity->getTableName(), dragStartPos[item], item->pos(),
                                                    tr("Move table \"%1\"").arg(entity->getTableName()));
                names << entity->getTableName();
            }
        }
    }

    ErdChange* change = ErdChange::normalizeChanges(changes, tr("Move tables: %1").arg(names.join(", ")));
    if (change)
        emit changeCreated(change);
}

void ErdView::createSimilar(ErdEntity* referenceEntity)
{
    SqliteCreateTablePtr newCreateTable = referenceEntity->getTableModel()->typeCloneShared<SqliteCreateTable>();
    QString newName = scene()->getNewEntityName(referenceEntity->getTableName(), 2);
    newCreateTable->table = newName;

    ErdEntity* entity = new ErdEntity(newCreateTable);
    entity->setExistingTable(false);
    scene()->placeNewEntity(entity, referenceEntity->pos());

    scene()->setFocusItem(entity, Qt::MouseFocusReason);
    entity->setSelected(true);
}

void ErdView::clearSelection()
{
    scene()->clearSelection();
    scene()->clearFocus();
}

void ErdView::setErdWindow(ErdWindow* newErdWindow)
{
    erdWindow = newErdWindow;
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
        case ErdView::Mode::AREA_SELECTING:
            dbg << "AREA_SELECTING";
            break;
    }
    return dbg;
}
