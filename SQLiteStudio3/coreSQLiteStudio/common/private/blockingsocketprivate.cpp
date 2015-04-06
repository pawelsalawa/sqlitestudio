#include "blockingsocketprivate.h"
#include <QTcpSocket>
#include <QCoreApplication>
#include <QTimer>
#include <QThread>

BlockingSocketPrivate::BlockingSocketPrivate()
{
    socket = new QTcpSocket(this);
    connect(socket, SIGNAL(disconnected()), this, SIGNAL(disconnected()));
}

BlockingSocketPrivate::~BlockingSocketPrivate()
{
}

void BlockingSocketPrivate::setError(QAbstractSocket::SocketError errorCode, const QString& errMsg)
{
    this->errorCode = errorCode;
    this->errorText = errMsg;
}

bool BlockingSocketPrivate::isConnected()
{
    return (socket->isOpen() && socket->state() == QAbstractSocket::ConnectedState);
}

QAbstractSocket::SocketError BlockingSocketPrivate::getErrorCode()
{
    return errorCode;
}

QString BlockingSocketPrivate::getErrorText()
{
    return errorText;
}

void BlockingSocketPrivate::handleSendCall(const QByteArray& bytes, bool& result)
{
    result = true;
    qint64 size = bytes.size();
    qint64 totalBytesSent = 0;
    qint64 bytesSent = 0;
    while (totalBytesSent < size)
    {
        bytesSent = socket->write(totalBytesSent == 0 ? bytes : bytes.mid(totalBytesSent));
        if (bytesSent < 0)
        {
            result = false;
            setError(socket->error(), socket->errorString());
            return;
        }
        totalBytesSent += bytesSent;
    }
}

void BlockingSocketPrivate::handleReadCall(qint64 count, int timeout, QByteArray& resultBytes, bool& result)
{
    resultBytes.clear();
    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

    QTimer timer;
    timer.setSingleShot(true);
    timer.setInterval(timeout);
    timer.start();

    while (resultBytes.size() < count && timer.isActive())
    {
        if (!isConnected())
        {
            qWarning() << "Blocking socket closed in the middle of reading.";
            result = false;
            setError(socket->error(), socket->errorString());
            return;
        }

        if (socket->bytesAvailable() == 0)
        {
            QThread::msleep(1);
            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
            continue;
        }

        resultBytes += socket->read(qMin(socket->bytesAvailable(), count));
    }

    result = (resultBytes.size() >= count && timer.isActive());
}

void BlockingSocketPrivate::handleConnectCall(const QString& host, int port, bool& result)
{
    result = true;
    if (isConnected())
        return;

    socket->connectToHost(host, port);
    if (!socket->waitForConnected())
    {
        result = false;
        setError(socket->error(), socket->errorString());
    }
}

void BlockingSocketPrivate::handleDisconnectCall()
{
    if (!isConnected())
        return;

    socket->abort();
    socket->close();

}

void BlockingSocketPrivate::handleIsConnectedCall(bool& connected)
{
    connected = isConnected();
}
