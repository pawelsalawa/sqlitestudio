#ifndef BLOCKINGSOCKETPRIVATE_H
#define BLOCKINGSOCKETPRIVATE_H

#include <QObject>
#include <QAbstractSocket>

class BlockingSocketPrivate : public QObject
{
        Q_OBJECT

    public:
        BlockingSocketPrivate();
        ~BlockingSocketPrivate();

        QString getErrorText();
        QAbstractSocket::SocketError getErrorCode();

    private:
        void createSocketIfNecessary();
        void setError(QAbstractSocket::SocketError errorCode, const QString& errMsg);
        bool isConnected();

        QAbstractSocket* socket = nullptr;
        QAbstractSocket::SocketError errorCode = QAbstractSocket::UnknownSocketError;
        QString errorText;

    private slots:
        void handleSendCall(const QByteArray& bytes, bool& result);
        void handleReadCall(qint64 count, int timeout, QByteArray& resultBytes, bool& result);
        void handleConnectCall(const QString& host, int port, bool& result);
        void handleDisconnectCall();
        void handleIsConnectedCall(bool& connected);

    signals:
        void disconnected();
};

#endif // BLOCKINGSOCKETPRIVATE_H
