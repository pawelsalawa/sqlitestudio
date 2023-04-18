
#ifndef ERDVIEW_H
#define ERDVIEW_H

#include <QGraphicsView>
#include <QPoint>
#include <QObject>

class ErdConnection;

class ErdView : public QGraphicsView
{
    public:
        ErdView(QWidget *parent = nullptr);

    protected:
        void mousePressEvent(QMouseEvent* event) override;
        void mouseMoveEvent(QMouseEvent* event) override;
        void mouseReleaseEvent(QMouseEvent* event) override;
        void mouseDoubleClickEvent(QMouseEvent* event) override;
        void keyPressEvent(QKeyEvent *event) override;
        void keyReleaseEvent(QKeyEvent *event) override;
        void focusOutEvent(QFocusEvent *event) override;
        void wheelEvent(QWheelEvent *event) override;

    private:
        void viewClicked(const QPoint& pos);
        QGraphicsItem* clickableItemAt(const QPoint& pos);
        void spacePressed();
        void spaceReleased();

        QGraphicsItem* selectedItem = nullptr;
        ErdConnection* currentConnection = nullptr;
        QPoint dragOffset;
        QPoint clickPos;
        bool spaceIsPressed = false;
};

#endif // ERDVIEW_H
