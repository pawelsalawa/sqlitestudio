#ifndef READWRITELOCKER_H
#define READWRITELOCKER_H

class QReadLocker;
class QWriteLocker;
class QReadWriteLock;

/**
 * @brief The ReadWriteLocker class
 * This class behaves pretty much like QReadLocker or QWriteLocker
 * (it actually uses those internally), except it can be either
 * of those and this is to be decided at the moment of creation.
 * Therefore the locker can work as read or write locker depending
 * on an external condition.
 * There's also a possibility to not lock anything as a third
 * choice of working mode, so this also can be decided
 * at construction moment.
 */
class ReadWriteLocker
{
    public:
        enum Mode
        {
            READ,
            WRITE,
            NONE
        };

        ReadWriteLocker(QReadWriteLock* lock, Mode mode);
        virtual ~ReadWriteLocker();

    private:
        QReadLocker* readLocker = nullptr;
        QWriteLocker* writeLocker = nullptr;
};

#endif // READWRITELOCKER_H
