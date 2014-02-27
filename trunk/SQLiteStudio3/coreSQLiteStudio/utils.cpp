#include "utils.h"

#include <QString>
#include <QSet>
#include <QVariant>
#include <QDateTime>

void initUtils()
{
    // No-op currently.
}

bool isXDigit(const QChar& c)
{
    return c.isDigit() || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

QChar charAt(const QString& str, int pos)
{
    if (pos < 0 || pos >= str.size())
        return QChar(0);

    return str[pos];
}

int rand(int min, int max)
{
    return qrand() % (max-min) + min;
}

const char* alphaNumChars = "abcdefghijklmnopqrstuvwxyz1234567890";
QString randStr(int length, bool numChars)
{
    int range = numChars ? 36 : 26;
    char* output = new char[length];
    for (int i =0; i < length; i++)
        output[i] = alphaNumChars[rand(0, range)];

    return QString::fromLatin1(output, length);
}

QString randBinStr(int length)
{
    char* output = new char[length];
    for (int i =0; i < length; i++)
        output[i] = rand(0, 256);

    return QString::fromLatin1(output, length);
}

QString randStrNotIn(int length, const QSet<QString> set)
{
    char* output = new char[length];
    QString outStr;

    do
    {
        for (int i =0; i < length; i++)
            output[i] = alphaNumChars[rand(0, 36)];

        outStr = QString::fromLatin1(output, length);
    }
    while (set.contains(outStr));

    return outStr;
}


Range::Range(qint64 from, qint64 to)
    :from(from), to(to)
{
}

bool Range::contains(qint64 position)
{
    return position >= from && position <= to;
}


QString generateUniqueName(const QString &baseName, const QStringList &existingNames)
{
    QString name = baseName;
    int i = 0;
    while (existingNames.contains(name))
        name = baseName+QString::number(i++);

    return name;
}

bool isNumeric(const QVariant& value)
{
    bool ok;
    value.toLongLong(&ok);
    if (ok)
        return true;

    value.toDouble(&ok);
    return ok;
}

QString rStrip(const QString& str)
{
    if (str.isNull())
        return str;

    for (int n = str.size() - 1; n >= 0; n--)
    {
        if (!str.at(n).isSpace())
            return str.left(n + 1);
    }
    return "";
}

int indexOf(const QStringList& list, const QString& value, Qt::CaseSensitivity cs)
{
    return indexOf(list, value, 0, cs);
}

int indexOf(const QStringList& list, const QString& value, int from, Qt::CaseSensitivity cs)
{
    if (cs == Qt::CaseSensitive)
        return list.indexOf(value, from);

    int cnt = list.size();
    for (int i = from; i < cnt; i++)
        if (list[i].compare(value, cs) == 0)
            return i;

    return -1;
}

QString pad(const QString& str, int length, const QChar& fillChar)
{
    if (str.length() >= abs(length))
        return str;

    QString result = str;
    QString fill = QString(fillChar).repeated(abs(length) - str.length());
    if (length >= 0)
        return result.append(fill);
    else
        return result.prepend(fill);
}

QString longest(const QStringList& strList)
{
    int max = 0;
    QString result;
    foreach (const QString str, strList)
    {
        if (str.size() > max)
        {
            result = str;
            max = str.size();
        }
    }
    return result;
}

QString shortest(const QStringList& strList)
{
    int max = INT_MAX;
    QString result;
    foreach (const QString str, strList)
    {
        if (str.size() < max)
        {
            result = str;
            max = str.size();
        }
    }
    return result;
}

QStringList applyMargin(const QString& str, int margin)
{
    QStringList lines;
    QString line;
    foreach (QString word, str.split(" "))
    {
        if (((line + word).length() + 1) > margin)
        {
            if (!line.isEmpty())
            {
                lines << line;
                line.clear();
            }

            while ((line + word).length() > margin)
            {
                line += word.left(margin);
                lines << line;
                word = word.mid(margin);
            }
        }

        if (!line.isEmpty())
            line += " ";

        line += word;

        if (line.endsWith("\n"))
        {
            lines << line.trimmed();
            line.clear();
        }
    }

    if (!line.isEmpty())
        lines << line;

    if (lines.size() == 0)
        lines << QString();

    return lines;
}

QDateTime toGregorian(double julianDateTime)
{
    int Z = (int)julianDateTime;
    double F = julianDateTime - Z;

    int A;
    if (Z < 2299161)
    {
        A = Z;
    }
    else
    {
        int alpha = (int)((Z - 1867216.25)/36524.25);
        A = Z + 1 + alpha - (int)(alpha / 4);
    }

    int B = A + 1524;
    int C = (int)((B - 122.1) / 365.25);
    int D = (int)(365.25 * C);
    int E = (int)((B-D) / 30.6001);
    int DD = B - D - (int)(30.6001 * E) + F;
    int MM = (E <= 13) ? E - 1 : E - 13;
    int YYYY = (MM <= 2) ? C - 4715 : C - 4716;

    int mmmBase = round(F * 86400000.0);
    int mmm = mmmBase % 1000;
    int ssBase = mmmBase / 1000;
    int ss = ssBase % 60;
    int mmBase = ssBase / 60;
    int mm = mmBase % 60;
    int hh = (mmBase / 60) + 12;
    if (hh >= 24)
    {
        hh -= 24;
        DD++;
    }

    QDateTime dateTime;
    dateTime.setDate(QDate(YYYY, MM, DD));
    dateTime.setTime(QTime(hh, mm, ss, mmm));
    return dateTime;
}

double toJulian(const QDateTime& gregDateTime)
{
    QDate date = gregDateTime.date();
    QTime time = gregDateTime.time();
    return toJulian(date.year(), date.month(), date.day(), time.hour(), time.minute(), time.second(), time.msec());
}

double toJulian(int year, int month, int day, int hour, int minute, int second, int msec)
{
    int a = (14 - month) / 12;
    int y = year + 4800 + a;
    int m = month + 12 * a - 3;

    // Julian Day
    int jnd = day + (153 * m + 2) / 5 + 365 * y + y / 4 - y / 100 + y / 400 - 32045;

    // Julian Day + Julian Time
    double jndt = jnd + (hour - 12.0) / 24.0 + minute / 1440.0 + second / 86400.0 + msec / 86400000.0;

    return jndt;
}

QString formatFileSize(quint64 size)
{
    quint64 bytes = size;
    quint64 kb = 0;
    quint64 mb = 0;
    quint64 gb = 0;

    QStringList words;
    if (bytes > (1024*1024*1024))
    {
        gb = bytes / (1024*1024*1024);
        bytes %= (1024*1024*1024);
        words << QString("%1GB").arg(gb);
    }

    if (bytes > (1024*1024))
    {
        mb = bytes / (1024*1024);
        bytes %= (1024*1024);
        words << QString("%1MB").arg(mb);
    }

    if (bytes > 1024)
    {
        kb = bytes / 1024;
        bytes %= 1024;
        words << QString("%1KB").arg(kb);
    }

    if (bytes > 0)
        words << QString("%1B").arg(bytes);

    return words.join(" ");
}

QString formatTimePeriod(int msecs)
{
    int hours = 0;
    int minutes = 0;
    int seconds = 0;

    QStringList words;
    if (msecs > (1000*60*60))
    {
        hours = msecs / (1000*60*60);
        msecs %= (1000*60*60);
        words << QString("%1h").arg(hours);
    }

    if (msecs > (1000*60))
    {
        minutes = msecs / (1000*60);
        msecs %= (1000*60);
        words << QString("%1m").arg(minutes);
    }

    if (msecs > (1000))
    {
        seconds = msecs / 1000;
        msecs %= 1000;
        words << QString("%1s").arg(seconds);
    }

    if (msecs > 0)
        words << QString("%1ms").arg(msecs);

    return words.join(" ");
}
