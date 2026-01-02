#ifndef ERDVIEW_H
#define ERDVIEW_H

#include "erdarrowitem.h"
#include <QGraphicsView>
#include <QPoint>
#include <QObject>
#include <QHash>
#include <QStack>
#include <QDebug>

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
            DRAGGING,
            CONNECTION_DRAFTING,
            PLACING_NEW_ENTITY,
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

        bool isDragging() const;
        bool isDraftingConnection() const;
        bool isPlacingNewEntity() const;
        void setOperatingMode(Mode mode);
        void pushOperatingMode(Mode mode);
        void popOperatingMode();

        static constexpr const char* CFG_KEY_ZOOM = "zoom";
        static constexpr const char* CFG_KEY_CENTER_POINT = "centerPoint";

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

        bool viewClicked(const QPoint& pos, Qt::MouseButton button);
        QGraphicsItem* clickableItemAt(const QPoint& pos);
        bool handleConnectionClick(const QPoint& pos, bool enableConnectionDrafting = false);
        void spacePressed();
        void spaceReleased();
        void handleSelectionOnMouseEvent(const QPoint& pos);
        void clearSelectedItems();
        void applyCursor(QIcon* icon);
        bool sameItemOnPositions(const QPoint& pos1, const QPoint& pos2);
        bool tolerateMicroMovesForClick();

        QList<QGraphicsItem*> selectedItems;
        QList<QGraphicsItem*> selectedMovableItems;
        QHash<QGraphicsItem*, QPoint> dragOffset;
        ErdConnection* draftConnection = nullptr;
        QPoint clickPos;
        qreal zoom = 1.0;
        KeyPressFilter* keyFilter = nullptr;
        bool centerPointRestored = false;
        QPointF centerPoint;
        Mode operatingMode = Mode::NORMAL;
        QStack<Mode> priorOperatingModeStack;

    public slots:
        void abortDraftConnection();
        void setDraftingConnectionMode(bool enabled);
        void insertNewEntity();
        void deleteSelectedItem();

    private slots:
        void resetZoom();
        void showItemToUser(QGraphicsItem* item);
        void leavingOperatingMode(ErdView::Mode mode);
        void enteringOperatingMode(ErdView::Mode mode);

    signals:
        void draftConnectionRemoved();
        void tableInsertionAborted();
        void newEntityPositionPicked(const QPointF& pos);
};

QDebug operator<<(QDebug dbg, ErdView::Mode value);

#endif // ERDVIEW_H
