#include "parsererror.h"
#include "token.h"

ParserError::ParserError(TokenPtr token, const QString &text)
{
    errorToken = token;
    message = text;
}

ParserError::ParserError(const QString &text)
{
    message = text;
}

QString &ParserError::getMessage()
{
    return message;
}

qint64 ParserError::getFrom()
{
    if (errorToken)
        return errorToken->start;
    else
        return -1;
}

qint64 ParserError::getTo()
{
    if (errorToken)
        return errorToken->end;
    else
        return -1;
}

TokenPtr ParserError::getErrorToken()
{
    return errorToken;
}

QString ParserError::toString()
{
    return QString("%1: %2").arg(errorToken->start).arg(message);
}
