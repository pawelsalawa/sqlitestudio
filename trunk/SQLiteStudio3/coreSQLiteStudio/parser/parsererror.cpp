#include "parsererror.h"
#include "token.h"

ParserError::ParserError(TokenPtr token, const QString &text)
{
    if (token)
    {
        start = token->start;
        end = token->end;
    }
    message = text;
}

ParserError::ParserError(qint64 start, qint64 end, const QString& text) :
    message(text),
    start(start),
    end(end)
{
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
    return start;
}

qint64 ParserError::getTo()
{
    return end;
}

QString ParserError::toString()
{
    return QString("%1: %2").arg(start).arg(message);
}
