#ifndef COMMITTABLE_H
#define COMMITTABLE_H

#include <QList>
#include <functional>

class Committable
{
    public:
        typedef std::function<bool(const QList<Committable*>& instances)> ConfirmFunction;

        Committable();
        virtual ~Committable();

        virtual bool isUncommited() const = 0;
        virtual QString getQuitUncommitedConfirmMessage() const = 0;

        static void init(ConfirmFunction confirmFunc);
        static bool canQuit();

    private:
        static ConfirmFunction confirmFunc;
        static QList<Committable*> instances;
};

#endif // COMMITTABLE_H
