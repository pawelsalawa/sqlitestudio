#include "spinnowheelfilter.h"
#include <QSpinBox>
#include <QEvent>

SpinNoWheelFilter::SpinNoWheelFilter(QObject* parent) : QObject(parent)
{
}

bool SpinNoWheelFilter::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::Wheel)
    {
        auto* spin = qobject_cast<QSpinBox*>(obj);
        if (spin)
        {
            event->ignore();
            return true;
        }
    }
    return QObject::eventFilter(obj, event);
}
