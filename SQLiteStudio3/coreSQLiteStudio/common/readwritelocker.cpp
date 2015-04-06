#include "readwritelocker.h"
#include "parser/lexer.h"
#include "common/utils_sql.h"
#include <QReadWriteLock>
#include <QReadLocker>
#include <QWriteLocker>
#include <QDebug>

ReadWriteLocker::ReadWriteLocker(QReadWriteLock* lock, Mode mode)
{
    init(lock, mode);
}

ReadWriteLocker::ReadWriteLocker(QReadWriteLock* lock, const QString& query, Dialect dialect, bool noLock)
{
    init(lock, getMode(query, dialect, noLock));
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

void ReadWriteLocker::init(QReadWriteLock* lock, ReadWriteLocker::Mode mode)
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

ReadWriteLocker::Mode ReadWriteLocker::getMode(const QString &query, Dialect dialect, bool noLock)
{
    if (noLock)
        return ReadWriteLocker::NONE;

    QueryAccessMode queryMode = getQueryAccessMode(query, dialect);
    switch (queryMode)
    {
        case QueryAccessMode::READ:
            return ReadWriteLocker::READ;
        case QueryAccessMode::WRITE:
            return ReadWriteLocker::WRITE;
    }

    qCritical() << "Unhandled query access mode:" << static_cast<int>(queryMode);
    return ReadWriteLocker::NONE;
}
