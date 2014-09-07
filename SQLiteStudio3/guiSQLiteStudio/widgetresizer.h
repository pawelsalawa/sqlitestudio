#ifndef WIDGETRESIZER_H
#define WIDGETRESIZER_H

#include "guiSQLiteStudio_global.h"
#include <QWidget>

class GUI_API_EXPORT WidgetResizer : public QWidget
{
        Q_OBJECT
    public:
        explicit WidgetResizer(const Qt::Orientation& orientation, QWidget *parent = 0);
        explicit WidgetResizer(QWidget *parent = 0);
        ~WidgetResizer();

        Qt::Orientation getOrientation() const;
        void setOrientation(const Qt::Orientation& value);

        int getWidth() const;
        void setWidth(int value);

        QWidget* getWidget() const;
        void setWidget(QWidget* value);

        QSize getWidgetMinimumSize() const;
        void setWidgetMinimumSize(const QSize& value);
        void setWidgetMinimumSize(int width, int height);

    protected:
        void mouseMoveEvent(QMouseEvent* event);
        void mousePressEvent(QMouseEvent* event);

    private:
        void init();
        void updateCursor();
        void updateWidth();
        void handleHorizontalMove(int position);
        void handleVerticalMove(int position);

        Qt::Orientation orientation = Qt::Vertical;
        int width = 4;
        QWidget* widget = nullptr;
        QPoint dragStartPosition;
        QSize dragStartSize;
        QSize widgetMinimumSize;
};

#endif // WIDGETRESIZER_H
