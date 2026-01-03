#include "deleteonfocusoutfilter.h"
#include <QFocusEvent>

void DeleteOnFocusOutFilter::ignoredReason(Qt::FocusReason reason)
{
    ignoredFocusReasons << reason;
}

bool DeleteOnFocusOutFilter::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::FocusOut)
    {
        QFocusEvent* fe = static_cast<QFocusEvent*>(event);
        if (ignoredFocusReasons.contains(fe->reason()))
            return QObject::eventFilter(obj, event);

        if (auto w = qobject_cast<QObject*>(obj))
            w->deleteLater();
    }
    return QObject::eventFilter(obj, event);
}
