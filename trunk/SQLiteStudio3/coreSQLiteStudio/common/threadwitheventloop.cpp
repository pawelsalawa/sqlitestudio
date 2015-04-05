#include "threadwitheventloop.h"

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

