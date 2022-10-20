#include "mouseshortcut.h"
#include <QMouseEvent>
#include <QApplication>

MouseShortcut::MouseShortcut(ClickType type, Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, QObject* parent)
    : QObject(parent), type(type), buttons(buttons), modifiers(modifiers)
{
}

bool MouseShortcut::eventFilter(QObject* object, QEvent* event)
{
    if ((event->type() == QEvent::MouseButtonPress && type == SingleClick && attributesMatch(event)) ||
        (event->type() == QEvent::MouseButtonDblClick && type == DoubleClick && attributesMatch(event)))
        emit activated();

    return QObject::eventFilter(object, event);
}

bool MouseShortcut::attributesMatch(QEvent* event)
{
    QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(event);
    if (!mouseEvent)
        return false;

    return (buttons.testFlag(mouseEvent->button())) && (modifiers == mouseEvent->modifiers());
}
