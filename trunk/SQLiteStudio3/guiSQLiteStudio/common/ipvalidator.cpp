#include "ipvalidator.h"
#include <QDebug>

IpValidator::IpValidator(QObject* parent) :
    QValidator(parent)
{

}

IpValidator::~IpValidator()
{

}

QValidator::State IpValidator::validate(QString& input, int&) const
{
    qDebug() << "input:" << input;
    QString reStr = "%1(\\d*)%1\\.%1(\\d*)%1\\.%1(\\d*)%1\\.%1(\\d*)%1";
    if (acceptWhiteSpaces)
    {
        if (whitespaceCharacter == ' ')
            reStr = reStr.arg("\\s*");
        else
            reStr = reStr.arg(whitespaceCharacter + QString("*"));
    }
    else
    {
        reStr = reStr.arg("");
    }

    QRegularExpression re(reStr);
    QRegularExpressionMatch match = re.match(input);
    if (!match.hasMatch())
        return Invalid;

    QString part;
    int value;
    bool ok;
    for (int i = 1; i <= 4; i++)
    {
        part = match.captured(i);
        if (part.isEmpty())
        {
            if (acceptEmptyParts)
                continue;
            else
                return Invalid;
        }

        value = part.toInt(&ok);
        if (!ok)
            return Invalid;

        if (value > 255 || value < 0)
            return Invalid;
    }

    return Acceptable;
}

bool IpValidator::getAcceptWhiteSpaces() const
{
    return acceptWhiteSpaces;
}

void IpValidator::setAcceptWhiteSpaces(bool value)
{
    acceptWhiteSpaces = value;
}
bool IpValidator::getAcceptEmptyParts() const
{
    return acceptEmptyParts;
}

void IpValidator::setAcceptEmptyParts(bool value)
{
    acceptEmptyParts = value;
}
QChar IpValidator::getWhitespaceCharacter() const
{
    return whitespaceCharacter;
}

void IpValidator::setWhitespaceCharacter(const QChar& value)
{
    whitespaceCharacter = value;
}



