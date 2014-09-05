#include "widgetresizer.h"
#include "common/unused.h"
#include <QMouseEvent>
#include <QDebug>
#include <QLayout>

WidgetResizer::WidgetResizer(QWidget *parent) :
    QWidget(parent)
{
    init();
}

WidgetResizer::~WidgetResizer()
{
}

WidgetResizer::WidgetResizer(const Qt::Orientation& orientation, QWidget* parent) :
    QWidget(parent), orientation(orientation)
{
    init();
}

void WidgetResizer::init()
{
    updateCursor();
    updateWidth();
    widgetMinimumSize = QSize(20, 20);
}

Qt::Orientation WidgetResizer::getOrientation() const
{
    return orientation;
}

void WidgetResizer::setOrientation(const Qt::Orientation& value)
{
    orientation = value;
    updateCursor();
}

void WidgetResizer::updateCursor()
{
    switch (orientation)
    {
        case Qt::Horizontal:
            setCursor(Qt::SplitHCursor);
            break;
        case Qt::Vertical:
            setCursor(Qt::SplitVCursor);
            break;
    }
}

void WidgetResizer::updateWidth()
{
    setMinimumSize(width, width);
}

void WidgetResizer::mousePressEvent(QMouseEvent* event)
{
    UNUSED(event);
    if (!widget)
        return;

    dragStartPosition = QCursor::pos();
    dragStartSize = widget->size();
}

void WidgetResizer::mouseMoveEvent(QMouseEvent* event)
{
    UNUSED(event);
    if (!widget)
        return;

    switch (orientation)
    {
        case Qt::Horizontal:
            handleHorizontalMove(QCursor::pos().x());
            break;
        case Qt::Vertical:
            handleVerticalMove(QCursor::pos().y());
            break;
    }
}

void WidgetResizer::handleHorizontalMove(int position)
{
    int newWidth = dragStartSize.width() + position - dragStartPosition.y();
    if (newWidth < widgetMinimumSize.width())
        return;

    widget->setFixedWidth(newWidth);
}

void WidgetResizer::handleVerticalMove(int position)
{
    int newHeight = dragStartSize.height() + position - dragStartPosition.y();
    if (newHeight < widgetMinimumSize.height())
        return;

    widget->setFixedHeight(newHeight);
}

int WidgetResizer::getWidth() const
{
    return width;
}

void WidgetResizer::setWidth(int value)
{
    width = value;
}

QWidget* WidgetResizer::getWidget() const
{
    return widget;
}

void WidgetResizer::setWidget(QWidget* value)
{
    widget = value;
}

QSize WidgetResizer::getWidgetMinimumSize() const
{
    return widgetMinimumSize;
}

void WidgetResizer::setWidgetMinimumSize(const QSize& value)
{
    widgetMinimumSize = value;
}

void WidgetResizer::setWidgetMinimumSize(int width, int height)
{
    widgetMinimumSize = QSize(width, height);
}
