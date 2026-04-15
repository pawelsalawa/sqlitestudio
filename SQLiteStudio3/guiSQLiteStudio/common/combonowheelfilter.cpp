#include "combonowheelfilter.h"
#include <QComboBox>
#include <QEvent>
#include <QAbstractItemView>

ComboNoWheelFilter::ComboNoWheelFilter(QObject* parent) : QObject(parent)
{
}

bool ComboNoWheelFilter::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::Wheel)
    {
        auto* combo = qobject_cast<QComboBox*>(obj);
        if (combo && !combo->view()->isVisible())
        {
            event->ignore();
            return true;
        }
    }
    return QObject::eventFilter(obj, event);
}
