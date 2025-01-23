#ifndef ERDVIEW_H
#define ERDVIEW_H

#include "erdarrowitem.h"
#include <QGraphicsView>
#include <QPoint>
#include <QObject>
#include <QHash>

class ErdConnection;
class ErdScene;

class ErdView : public QGraphicsView
{
    Q_OBJECT

    public:
        ErdView(QWidget *parent = nullptr);
        ~ErdView();

        void setScene(ErdScene *scene);
        ErdScene *scene() const;
        bool isSpacePressed() const;
        qreal getZoom() const;
        void applyConfig(const QHash<QString, QVariant>& erdConfig);
        QHash<QString, QVariant> getConfig();
        void applyZoomRatio(qreal ratio);

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
        void spacePressed();
        void spaceReleased();
        void handleSelectionOnMouseEvent(const QPoint& pos);
        void clearSelectedItems();

        QList<QGraphicsItem*> selectedItems;
        QList<QGraphicsItem*> selectedMovableItems;
        QHash<QGraphicsItem*, QPoint> dragOffset;
        ErdConnection* draftConnection = nullptr;
        bool draftingConnectionMode = false;
        QPoint clickPos;
        qreal zoom = 1.0;
        bool spaceIsPressed = false;
        KeyPressFilter* keyFilter = nullptr;
        bool centerPointRestored = false;
        QPointF centerPoint;

    public slots:
        void abortDraftConnection();
        void setDraftingConnectionMode(bool enabled);

    private slots:
        void resetZoom();
        void showItemToUser(QGraphicsItem* item);

    signals:
        void draftConnectionRemoved();
};

#endif // ERDVIEW_H
