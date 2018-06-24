#ifndef LAZYTRIGGER_H
#define LAZYTRIGGER_H

#include "coreSQLiteStudio_global.h"
#include <QObject>
#include <functional>

class QTimer;

class API_EXPORT LazyTrigger : public QObject
{
    Q_OBJECT

    public:
        typedef std::function<bool()> Condition;

        LazyTrigger(int delay, QObject* parent = nullptr, const char* slot = nullptr);
        LazyTrigger(int delay, Condition condition, QObject* parent = nullptr, const char* slot = nullptr);

        void setDelay(int delay);

    private:
        QTimer* timer = nullptr;
        Condition condition = nullptr;

    public slots:
        void schedule();
        void cancel();

    signals:
        void triggered();
};

#endif // LAZYTRIGGER_H
