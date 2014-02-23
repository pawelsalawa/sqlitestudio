#ifndef NULLDEVICE_H
#define NULLDEVICE_H

#include <QIODevice>

class NullDevice : public QIODevice
{
    public:
        explicit NullDevice(QObject *parent = 0);

        qint64 readData(char * data, qint64 maxSize);
        qint64 writeData(const char * data, qint64 maxSize);
};

#endif // NULLDEVICE_H
