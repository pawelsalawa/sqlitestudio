#include "bigdec.h"
#include <QChar>
#include <QtGlobal>

BigDec::BigDec() : digits("0"), scale(0), negative(false)
{}

BigDec::BigDec(const QString &s)
{
    fromString(s);
}

void BigDec::fromString(const QString &s)
{
    digits.clear();
    scale = 0;
    negative = false;

    bool afterDot = false;
    int i = 0;

    if (!s.isEmpty() && (s[0] == '-' || s[0] == '+'))
    {
        negative = (s[0] == '-');
        i++;
    }

    for (; i < s.size(); ++i)
    {
        QChar c = s[i];
        if (c == '.')
        {
            if (afterDot) qFatal("Invalid number format");
            afterDot = true;
        }
        else if (c.isDigit())
        {
            digits.append(c);
            if (afterDot) scale++;
        }
        else
        {
            qFatal("Invalid character in number");
        }
    }

    if (digits.isEmpty())
    {
        digits = "0";
        scale = 0;
        negative = false;
    }

    normalize();
}

BigDec &BigDec::operator+=(const QString &otherStr)
{
    BigDec other(otherStr);

    QString a = digits;
    QString b = other.digits;
    int scaleA = scale;
    int scaleB = other.scale;

    alignScales(a, scaleA, b, scaleB);

    if (negative == other.negative)
    {
        digits = addStrings(a, b);
    }
    else
    {
        int cmp = compareAbs(a, b);
        if (cmp == 0)
        {
            digits = "0";
            scale = 0;
            negative = false;
            return *this;
        }
        else if (cmp > 0)
        {
            digits = subStrings(a, b);
        }
        else
        {
            digits = subStrings(b, a);
            negative = other.negative;
        }
    }

    scale = scaleA;
    normalize();
    return *this;
}

QString BigDec::toString() const
{
    QString result;
    if (scale == 0)
    {
        result = digits;
    }
    else if (digits.size() <= scale)
    {
        result = "0.";
        result += QString(scale - digits.size(), QChar('0'));
        result += digits;
    }
    else
    {
        result = digits;
        result.insert(result.size() - scale, '.');
    }

    if (negative && result != "0")
        result.prepend('-');

    return result;
}

BigDec &BigDec::operator+=(double value)
{
    return (*this += QString::number(value, 'g', 17));
}

BigDec &BigDec::operator+=(int value)
{
    return (*this += QString::number(value));
}

BigDec &BigDec::operator+=(uint value)
{
    return (*this += QString::number(value));
}

BigDec &BigDec::operator+=(qlonglong value)
{
    return (*this += QString::number(value));
}

BigDec &BigDec::operator+=(qulonglong value)
{
    return (*this += QString::number(value));
}

QString BigDec::stripLeadingZeros(const QString &s)
{
    int i = 0;
    while (i + 1 < s.size() && s[i] == QChar('0'))
        i++;

    return s.mid(i);
}

QString BigDec::stripTrailingZeros(QString s, int &scale)
{
    while (!s.isEmpty() && s.back() == QChar('0') && scale > 0)
    {
        s.chop(1);
        scale--;
    }
    if (s.isEmpty())
    {
        s = "0";
        scale = 0;
    }
    return s;
}

void BigDec::alignScales(QString &a, int &scaleA, QString &b, int &scaleB)
{
    if (scaleA < scaleB)
    {
        a.append(QString(scaleB - scaleA, QChar('0')));
        scaleA = scaleB;
    }
    else if (scaleB < scaleA)
    {
        b.append(QString(scaleA - scaleB, QChar('0')));
        scaleB = scaleA;
    }
}

int BigDec::compareAbs(const QString &a, const QString &b)
{
    if (a.size() != b.size())
        return a.size() < b.size() ? -1 : 1;

    return a.compare(b);
}

QString BigDec::addStrings(const QString &a, const QString &b)
{
    QString res;
    res.reserve(qMax(a.size(), b.size()) + 1);

    int carry = 0;
    int i = a.size() - 1;
    int j = b.size() - 1;

    while (i >= 0 || j >= 0 || carry)
    {
        int da = (i >= 0 ? a[i--].digitValue() : 0);
        int db = (j >= 0 ? b[j--].digitValue() : 0);
        int sum = da + db + carry;
        carry = sum / 10;
        res.append(QChar('0' + (sum % 10)));
    }

    std::reverse(res.begin(), res.end());
    return res;
}

QString BigDec::subStrings(const QString &a, const QString &b)
{
    QString res;
    res.reserve(a.size());

    int borrow = 0;
    int i = a.size() - 1;
    int j = b.size() - 1;

    while (i >= 0)
    {
        int da = a[i--].digitValue() - borrow;
        int db = (j >= 0 ? b[j--].digitValue() : 0);

        if (da < db)
        {
            da += 10;
            borrow = 1;
        }
        else
        {
            borrow = 0;
        }

        res.append(QChar('0' + (da - db)));
    }

    std::reverse(res.begin(), res.end());
    return res;
}
void BigDec::normalize()
{
    digits = stripLeadingZeros(digits);
    digits = stripTrailingZeros(digits, scale);

    if (digits == "0")
    {
        negative = false;
        scale = 0;
    }
}
