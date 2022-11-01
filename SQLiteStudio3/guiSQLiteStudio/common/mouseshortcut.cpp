#include "mouseshortcut.h"
#include <QMouseEvent>
#include <QApplication>
#include <QDebug>

MouseShortcut::MouseShortcut(ClickType type, Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, QObject* parent)
    : QObject(parent), type(type), buttons(buttons), modifiers(modifiers)
{
}

MouseShortcut* MouseShortcut::forWheel(Qt::KeyboardModifiers modifiers, QObject* parent)
{
    return new MouseShortcut(Wheel, Qt::MouseButtons(), modifiers, parent);
}

MouseShortcut* MouseShortcut::forWheel(Qt::KeyboardModifiers modifiers, QObject* parent, const char* slot)
{
    MouseShortcut* instance = new MouseShortcut(Wheel, Qt::MouseButtons(), modifiers, parent);
    connect(instance, SIGNAL(wheelActivated(int)), parent, slot);
    parent->installEventFilter(instance);
    return instance;
}

MouseShortcut* MouseShortcut::forWheel(Qt::KeyboardModifiers modifiers, QObject* receiver, const char* slot, QObject* parent)
{
    MouseShortcut* instance = new MouseShortcut(Wheel, Qt::MouseButtons(), modifiers, parent);
    connect(instance, SIGNAL(wheelActivated(int)), receiver, slot);
    parent->installEventFilter(instance);
    return instance;
}

bool MouseShortcut::eventFilter(QObject* object, QEvent* event)
{
    if ((event->type() == QEvent::MouseButtonPress && type == SingleClick && attributesMatch(event)) ||
        (event->type() == QEvent::MouseButtonDblClick && type == DoubleClick && attributesMatch(event)))
    {
        emit activated();
        return true;
    }

    if (event->type() == QEvent::Wheel && type == Wheel)
    {
        QWheelEvent* wheelEvent = dynamic_cast<QWheelEvent*>(event);
        if (modifiers == wheelEvent->modifiers())
        {
            emit wheelActivated(wheelEvent->angleDelta().y());
            return true;
        }
    }

    return QObject::eventFilter(object, event);
}

bool MouseShortcut::attributesMatch(QEvent* event)
{
    QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(event);
    if (!mouseEvent)
        return false;

    return (buttons.testFlag(mouseEvent->button())) && (modifiers == mouseEvent->modifiers());
}
