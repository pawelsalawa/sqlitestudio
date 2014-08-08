#include "extaction.h"
#include <QDebug>
#include <QShortcutEvent>

ExtAction::ExtAction(QObject *parent) :
    QAction(parent)
{
}

ExtAction::ExtAction(const QString& text, QObject* parent) :
    QAction(text, parent)
{
}

ExtAction::ExtAction(const QIcon& icon, const QString& text, QObject* parent) :
    QAction(icon, text, parent)
{
}

bool ExtAction::event(QEvent* e)
{
    // This implementation code comes mostly from Qt 5.1.0,
    // but it was modified to handle ambiguous shortcuts.
    if (e->type() == QEvent::Shortcut)
    {
        activate(Trigger);
        return true;
    }
    return QObject::event(e);
}
