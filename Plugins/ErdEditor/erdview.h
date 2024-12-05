
#ifndef ERDVIEW_H
#define ERDVIEW_H

#include <QGraphicsView>
#include <QPoint>
#include <QObject>
#include <QHash>

class ErdConnection;
class ErdScene;

class ErdView : public QGraphicsView
{
    public:
        ErdView(QWidget *parent = nullptr);
        ~ErdView();

        void setScene(ErdScene *scene);
        ErdScene *scene() const;
        bool isSpacePressed() const;

    protected:
        void mousePressEvent(QMouseEvent* event) override;
        void mouseMoveEvent(QMouseEvent* event) override;
        void mouseReleaseEvent(QMouseEvent* event) override;
        void mouseDoubleClickEvent(QMouseEvent* event) override;
        void focusOutEvent(QFocusEvent *event) override;
        void wheelEvent(QWheelEvent *event) override;

    private:
        class KeyPressFilter : public QObject {
            public:
                KeyPressFilter(ErdView* view);

            protected:
                bool eventFilter(QObject *obj, QEvent *event) override;

            private:
                ErdView* view = nullptr;
        };

        void viewClicked(const QPoint& pos, Qt::MouseButton button);
        QGraphicsItem* clickableItemAt(const QPoint& pos);
        void spacePressed();
        void spaceReleased();
        void handleSelectionOnMouseEvent(const QPoint& pos);

        QList<QGraphicsItem*> selectedItems;
        QList<QGraphicsItem*> selectedMovableItems;
        QHash<QGraphicsItem*, QPoint> dragOffset;
        ErdConnection* draftConnection = nullptr;
        QPoint clickPos;
        qreal zoom = 1.0;
        bool spaceIsPressed = false;
        KeyPressFilter* keyFilter = nullptr;

    private slots:
        void resetZoom();
};

#endif // ERDVIEW_H
