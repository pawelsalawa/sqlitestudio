#ifndef THREADWITHEVENTLOOP_H
#define THREADWITHEVENTLOOP_H

#include <QObject>
#include <QThread>

class ThreadWithEventLoop : public QThread
{
        Q_OBJECT

    public:
        ThreadWithEventLoop(QObject* parent = nullptr);
        ~ThreadWithEventLoop();

    protected:
        void run() Q_DECL_OVERRIDE;
};

#endif // THREADWITHEVENTLOOP_H
