#include "lazytrigger.h"
#include <QTimer>

LazyTrigger::LazyTrigger(int delay, QObject* parent, const char* slot) :
    QObject(parent)
{
    timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->setInterval(delay);
    connect(timer, &QTimer::timeout, this, &LazyTrigger::triggered);
    if (slot)
        connect(timer, SIGNAL(timeout()), parent, slot);
}

LazyTrigger::LazyTrigger(int delay, LazyTrigger::Condition condition, QObject* parent, const char* slot) :
    LazyTrigger(delay, parent, slot)
{
    this->condition = condition;
}

void LazyTrigger::setDelay(int delay)
{
    timer->setInterval(delay);
}

void LazyTrigger::schedule()
{
    timer->stop();

    if (!condition || condition())
        timer->start();
}

void LazyTrigger::cancel()
{
    timer->stop();
}
