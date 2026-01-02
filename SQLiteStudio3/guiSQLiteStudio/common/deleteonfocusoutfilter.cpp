#include "deleteonfocusoutfilter.h"
#include <QFocusEvent>

bool DeleteOnFocusOutFilter::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::FocusOut)
    {
        if (auto w = qobject_cast<QObject*>(obj))
            w->deleteLater();
    }
    return QObject::eventFilter(obj, event);
}
