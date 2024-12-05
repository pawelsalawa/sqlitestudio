#include "immediatetooltip.h"
#include "common/unused.h"
#include <QEnterEvent>
#include <QToolTip>
#include <QWidget>
#include <QDebug>

ImmediateTooltip::ImmediateTooltip(QWidget* parent)
    : QObject(parent)
{
    parent->setMouseTracking(true);
    parent->installEventFilter(this);
}

bool ImmediateTooltip::eventFilter(QObject* obj, QEvent* event)
{
    UNUSED(obj);
    switch (event->type())
    {
        case QEvent::Enter:
        {
            QEnterEvent* e = dynamic_cast<QEnterEvent*>(event);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            QToolTip::showText(e->globalPos(), toolTip);
#else
            QToolTip::showText(e->globalPosition().toPoint(), toolTip);
#endif
            break;
        }
        case QEvent::Leave:
            QToolTip::hideText();
            break;
        default:
            break;
    }
    return false;
}

const QString& ImmediateTooltip::getToolTip() const
{
    return toolTip;
}

void ImmediateTooltip::setToolTip(const QString& newToolTip)
{
    toolTip = newToolTip;
}
