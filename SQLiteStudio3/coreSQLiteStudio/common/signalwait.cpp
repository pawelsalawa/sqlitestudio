#include "signalwait.h"
#include <QCoreApplication>
#include <QTime>

SignalWait::SignalWait(QObject* object, const char* signal) :
    QObject()
{
    connect(object, signal, this, SLOT(handleSignal()));
}

bool SignalWait::wait(int msTimeout)
{
    QTime timer(0, 0, 0, msTimeout);
    timer.start();
    while (!called && timer.elapsed() < msTimeout)
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

    return called;
}

void SignalWait::reset()
{
    called = false;
}

void SignalWait::handleSignal()
{
    called = true;
}
