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
    while (!called && !failed && timer.elapsed() < msTimeout)
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

    if (failed)
        return false;

    return called;
}

void SignalWait::reset()
{
    called = false;
}

void SignalWait::addFailSignal(QObject* object, const char* signal)
{
    connect(object, signal, this, SLOT(handleFailSignal()));
}

void SignalWait::handleSignal()
{
    called = true;
}

void SignalWait::handleFailSignal()
{
    failed = true;
}
