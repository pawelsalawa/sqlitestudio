#ifndef THREADEDSOCKET_H
#define THREADEDSOCKET_H

#include <QObject>
#include <QAbstractSocket>
#include <QMutex>

class BlockingSocketPrivate;
class ThreadWithEventLoop;

class BlockingSocket : public QObject
{
        Q_OBJECT

    public:
        BlockingSocket(QObject* parent = nullptr);
        ~BlockingSocket();

        QString getErrorText();
        QAbstractSocket::SocketError getErrorCode();
        bool connectToHost(const QString& host, int port);
        void disconnectFromHost();
        bool isConnected();
        bool send(const QByteArray& bytes);
        QByteArray read(qint64 count, int timeout = 30000, bool* ok = nullptr);
        void quit();
        void exit();

    private:
        ThreadWithEventLoop* socketThread = nullptr;
        BlockingSocketPrivate* socket = nullptr;
        QMutex socketOperationMutex;

    signals:
        void callForConnect(const QString& host, int port, bool& result);
        void callForDisconnect();
        void callForIsConnected(bool& connected);
        void callForSend(const QByteArray& bytes, bool& result);
        void callForRead(qint64 count, int timeout, QByteArray& resultBytes, bool& result);
        void disconnected();
};

#endif // THREADEDSOCKET_H
