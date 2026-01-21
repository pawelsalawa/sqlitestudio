#ifndef ERDVIEW_H
#define ERDVIEW_H

#include "erdarrowitem.h"
#include <QGraphicsView>
#include <QPoint>
#include <QObject>
#include <QHash>
#include <QStack>
#include <QDebug>

class ErdChange;
class ErdConnection;
class ErdScene;
class Db;

class ErdView : public QGraphicsView
{
    Q_OBJECT

    public:
        enum class Mode
        {
            NORMAL,
            DRAGGING_VIEW,
            CONNECTION_DRAFTING,
            PLACING_NEW_ENTITY,
            AREA_SELECTING,
        };

        ErdView(QWidget *parent = nullptr);
        ~ErdView();

        void setScene(ErdScene *scene);
        ErdScene *scene() const;
        Db* getDb() const;
        qreal getZoom() const;
        void applyConfig(const QHash<QString, QVariant>& erdConfig);
        QHash<QString, QVariant> getConfig();
        void applyZoomRatio(qreal ratio);

        bool isDraggingView() const;
        bool isDraftingConnection() const;
        bool isPlacingNewEntity() const;
        bool isNormalMode() const;
        void setOperatingMode(Mode mode);
        void pushOperatingMode(Mode mode);
        void popOperatingMode();

        static constexpr const char* CFG_KEY_ZOOM = "zoom";
        static constexpr const char* CFG_KEY_CENTER_POINT = "centerPoint";

        QPointF getLastClickPos() const;

    protected:
        void mousePressEvent(QMouseEvent* event) override;
        void mouseMoveEvent(QMouseEvent* event) override;
        void mouseReleaseEvent(QMouseEvent* event) override;
        void mouseDoubleClickEvent(QMouseEvent* event) override;
        void focusOutEvent(QFocusEvent *event) override;
        void wheelEvent(QWheelEvent *event) override;
        bool event(QEvent *event) override;

    private:
        class KeyPressFilter : public QObject {
            public:
                KeyPressFilter(ErdView* view);

            protected:
                bool eventFilter(QObject *obj, QEvent *event) override;

            private:
                ErdView* view = nullptr;
        };

        void mousePressedNormal(QMouseEvent* event);
        void mousePressedDraggingView(QMouseEvent* event);
        void mousePressedConnectionDrafting(QMouseEvent* event);
        void mousePressedPlacingNewEntity(QMouseEvent* event);
        void mousePressedAreaSelecting(QMouseEvent* event);

        void mouseMovedNormal(QMouseEvent* event);
        void mouseMovedDraggingView(QMouseEvent* event);
        void mouseMovedConnectionDrafting(QMouseEvent* event);
        void mouseMovedPlacingNewEntity(QMouseEvent* event);
        void mouseMovedAreaSelecting(QMouseEvent* event);

        void mouseReleasedNormal(QMouseEvent* event);
        void mouseReleasedDraggingView(QMouseEvent* event);
        void mouseReleasedConnectionDrafting(QMouseEvent* event);
        void mouseReleasedPlacingNewEntity(QMouseEvent* event);
        void mouseReleasedAreaSelecting(QMouseEvent* event);

        QGraphicsItem* clickableItemAt(const QPoint& pos);
        void spacePressed();
        void spaceReleased();
        void applyCursor(QIcon* icon);
        void startDragBySpace();
        void endDragBySpace();
        void itemsPotentiallyMoved();

        QHash<QGraphicsItem*, QPointF> dragStartPos;
        ErdConnection* draftConnection = nullptr;
        QPointF lastClickPos;
        qreal zoom = 1.0;
        KeyPressFilter* keyFilter = nullptr;
        bool centerPointRestored = false;
        QPointF centerPoint;
        Mode operatingMode = Mode::NORMAL;
        QStack<Mode> priorOperatingModeStack;

    public slots:
        void setDraftingConnectionMode(bool enabled);
        void abortDraftConnection();
        void insertNewEntity();
        void deleteSelectedItem();
        void handleVisibilityRequest(const QRectF& rect);
        void resetZoom();

    private slots:
        void showItemToUser(QGraphicsItem* item);
        void leavingOperatingMode(ErdView::Mode mode);
        void enteringOperatingMode(ErdView::Mode mode);

    signals:
        void draftConnectionRemoved();
        void tableInsertionAborted();
        void newEntityPositionPicked(const QPointF& pos);
        void changeCreated(ErdChange* change);
};

QDebug operator<<(QDebug dbg, ErdView::Mode value);

#endif // ERDVIEW_H
