#include "blockingsocket.h"
#include "common/global.h"
#include "common/threadwitheventloop.h"
#include "common/private/blockingsocketprivate.h"

BlockingSocket::BlockingSocket(QObject* parent) :
    QObject(parent)
{
    socketThread = new ThreadWithEventLoop;
    socket = new BlockingSocketPrivate;
    socket->moveToThread(socketThread);

    connect(socketThread, &QThread::finished, socket, &QObject::deleteLater);
    connect(socketThread, &QThread::finished, socketThread, &QObject::deleteLater);
    connect(this, SIGNAL(callForSend(QByteArray,bool&)), socket, SLOT(handleSendCall(QByteArray,bool&)), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(callForRead(qint64,int,QByteArray&,bool&)), socket, SLOT(handleReadCall(qint64,int,QByteArray&,bool&)), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(callForConnect(QString,int,bool&)), socket, SLOT(handleConnectCall(QString,int,bool&)), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(callForDisconnect()), socket, SLOT(handleDisconnectCall()), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(callForIsConnected(bool&)), socket, SLOT(handleIsConnectedCall(bool&)), Qt::BlockingQueuedConnection);

    socketThread->start();
}

BlockingSocket::~BlockingSocket()
{
    socketThread->quit();
}

QAbstractSocket::SocketError BlockingSocket::getErrorCode()
{
    return socket->getErrorCode();
}

QString BlockingSocket::getErrorText()
{
    return socket->getErrorText();
}

bool BlockingSocket::connectToHost(const QString& host, int port)
{
    bool res = false;
    emit callForConnect(host, port, res);
    return res;
}

void BlockingSocket::disconnectFromHost()
{
    emit callForDisconnect();
}

bool BlockingSocket::isConnected()
{
    bool res = false;
    emit callForIsConnected(res);
    return res;
}

bool BlockingSocket::send(const QByteArray& bytes)
{
    bool res = false;
    emit callForSend(bytes, res);
    return res;
}

QByteArray BlockingSocket::read(qint64 count, int timeout, bool* ok)
{
    bool res = false;
    QByteArray bytes;
    emit callForRead(count, timeout, bytes, res);
    if (ok)
        *ok = res;

    return bytes;
}

void BlockingSocket::quit()
{
    socketThread->quit();
}

void BlockingSocket::exit()
{
    socketThread->quit();
}
