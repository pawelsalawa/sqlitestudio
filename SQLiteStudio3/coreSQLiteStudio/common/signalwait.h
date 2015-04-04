#ifndef SIGNALWAIT_H
#define SIGNALWAIT_H

#include <QObject>

class SignalWait : public QObject
{
        Q_OBJECT

    public:
        SignalWait(QObject *object, const char *signal);

        bool wait(int msTimeout);
        void reset();

    private:
        bool called = false;

    private slots:
        void handleSignal();
};

#endif // SIGNALWAIT_H
