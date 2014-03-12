#include "readwritelocker.h"
#include <QReadWriteLock>
#include <QReadLocker>
#include <QWriteLocker>

ReadWriteLocker::ReadWriteLocker(QReadWriteLock* lock, Mode mode)
{
    switch (mode)
    {
        case ReadWriteLocker::READ:
            readLocker = new QReadLocker(lock);
            break;
        case ReadWriteLocker::WRITE:
            writeLocker = new QWriteLocker(lock);
            break;
        case ReadWriteLocker::NONE:
            // Nothing to lock.
            break;
    }
}

ReadWriteLocker::~ReadWriteLocker()
{
    if (readLocker)
    {
        delete readLocker;
        readLocker = nullptr;
    }

    if (writeLocker)
    {
        delete writeLocker;
        writeLocker = nullptr;
    }
}
