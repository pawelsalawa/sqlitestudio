#include "threadwitheventloop.h"
#include <QDebug>

ThreadWithEventLoop::ThreadWithEventLoop(QObject* parent) :
    QThread(parent)
{
}

ThreadWithEventLoop::~ThreadWithEventLoop()
{
}

void ThreadWithEventLoop::run()
{
    exec();
}

