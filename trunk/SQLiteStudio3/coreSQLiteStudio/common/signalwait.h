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
        void addFailSignal(QObject *object, const char *signal);

    private:
        bool called = false;
        bool failed = false;

    private slots:
        void handleSignal();
        void handleFailSignal();
};

#endif // SIGNALWAIT_H
