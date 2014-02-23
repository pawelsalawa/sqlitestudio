#include "returncode.h"

ReturnCode::ReturnCode()
    : retCode(0)
{
}

ReturnCode::ReturnCode(QString errorMessage)
    : retCode(0)
{
    errorMessages << errorMessage;
}

ReturnCode::ReturnCode(quint16 code, QString errorMessage)
    : retCode(code)
{
    errorMessages << errorMessage;
}

void ReturnCode::addMessage(QString errorMessage)
{
    errorMessages << errorMessage;
}

bool ReturnCode::isOk()
{
    return retCode != 0;
}

QList<QString> &ReturnCode::getMessages()
{
    return errorMessages;
}

QString ReturnCode::message()
{
    if (errorMessages.size() > 0)
        return errorMessages.at(0);
    else
        return QString::null;
}

void ReturnCode::setCode(quint16 code)
{
    retCode = code;
}

quint16 ReturnCode::code()
{
    return retCode;
}
