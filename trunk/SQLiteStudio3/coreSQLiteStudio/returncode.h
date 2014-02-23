#ifndef RETURNCODE_H
#define RETURNCODE_H

#include <QList>
#include <QString>

class ReturnCode
{
    private:
        quint16 retCode;
        QList<QString> errorMessages;

    public:
        ReturnCode();
        explicit ReturnCode(QString errorMessage);
        ReturnCode(quint16 code, QString errorMessage);

        void addMessage(QString errorMessage);
        bool isOk();
        QList<QString>& getMessages();
        QString message();
        void setCode(quint16 code);
        quint16 code();
};

#endif // RETURNCODE_H
