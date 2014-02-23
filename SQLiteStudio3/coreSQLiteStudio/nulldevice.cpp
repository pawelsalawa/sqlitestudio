#include "nulldevice.h"

NullDevice::NullDevice(QObject *parent) :
    QIODevice(parent)
{
}

qint64 NullDevice::readData(char *data, qint64 maxSize)
{
    (void)(data); // slicence unused var
    (void)(maxSize); // slicence unused var
    return 0;
}

qint64 NullDevice::writeData(const char *data, qint64 maxSize)
{
    (void)(data); // slicence unused var
    return maxSize;
}
