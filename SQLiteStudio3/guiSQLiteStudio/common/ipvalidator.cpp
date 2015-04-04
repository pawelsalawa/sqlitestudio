#include "ipvalidator.h"
#include <QDebug>

QString IpValidator::reStr = "^%1(\\d%2)%1\\.%1(\\d%2)%1\\.%1(\\d%2)%1\\.%1(\\d%2)%1$";

IpValidator::IpValidator(QObject* parent) :
    QValidator(parent)
{
}

IpValidator::~IpValidator()
{
}

QValidator::State IpValidator::validate(QString& input, int&) const
{
    QString regexp = getPattern(acceptWhiteSpaces, false, whitespaceCharacter);
    QRegularExpression re(regexp);
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

bool IpValidator::check(const QString& input, bool acceptWhiteSpaces)
{
    QString regexp = getPattern(acceptWhiteSpaces, true, ' ');
    QRegularExpression re(regexp);
    return re.match(input).hasMatch();
}

QString IpValidator::getPattern(bool acceptWhiteSpaces, bool requireFull, QChar whitespaceCharacter)
{
    QString countChar = requireFull ? "+" : "*";
    if (acceptWhiteSpaces)
    {
        if (whitespaceCharacter == ' ')
            return reStr.arg("\\s*", countChar);
        else
            return reStr.arg(whitespaceCharacter + QString("*"), countChar);
    }
    else
        return reStr.arg("", countChar);
}
