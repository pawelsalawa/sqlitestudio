#include "common/utils.h"
#include "common/global.h"
#include "dbobjecttype.h"
#include "rsa/RSA.h"
#include "common/compatibility.h"
#include <QString>
#include <QStringConverter>
#include <QStringEncoder>
#include <QStringDecoder>
#include <QSet>
#include <QVariant>
#include <QDateTime>
#include <QSysInfo>
#include <QDebug>
#include <QRegularExpression>
#include <QDir>
#include <QByteArray>
#include <QBitArray>
#include <QDataStream>
#include <QRandomGenerator>
#include <QThreadPool>

#ifdef Q_OS_LINUX
#include <sys/utsname.h>

#include <QFileInfo>
#endif

#ifdef Q_OS_WIN
#include <windows.h>
#include <tchar.h>

typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
bool is64BitWindows()
{
#if defined(_WIN64)
    return true;  // 64-bit programs run only on Win64
#elif defined(_WIN32)
    // 32-bit programs run on both 32-bit and 64-bit Windows
    // so must sniff
    BOOL f64 = false;
    LPFN_ISWOW64PROCESS fnIsWow64Process;

    fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(GetModuleHandle(TEXT("kernel32")), "IsWow64Process");
    if (fnIsWow64Process)
    {
        return fnIsWow64Process(GetCurrentProcess(), &f64) && f64;
    }
    return false;
#else
    return true; // Win64 does not support Win16
#endif
}
#endif

void initUtils()
{
    qRegisterMetaType<QList<int>>("QList<int>");
    qRegisterMetaType<DbObjectType>("DbObjectType");
    qRegisterMetaType<QList<QPair<QString, QString>>>("QList<QPair<QString, QString>>");
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
    return QRandomGenerator::system()->generate() % (max-min) + min;
}

QString randStr(int length, bool numChars, bool whiteSpaces)
{
    static const char* alphaNumChars = " abcdefghijklmnopqrstuvwxyz1234567890";
    int start = 1;
    int range = start + (numChars ? 36 : 26);

    if (whiteSpaces)
    {
        start--;
        range++;
    }

    QString output = "";
    for (int i = 0; i < length; i++)
        output += alphaNumChars[rand(start, range)];

    return output;
}

QString randStr(int length, const QString& charCollection)
{
    int range = charCollection.size();
    QString output = "";
    for (int i = 0; i < length; i++)
        output += charCollection[rand(0, range)];

    return output;
}

QString randBinStr(int length)
{
    char* output = new char[length];
    for (int i =0; i < length; i++)
        output[i] = rand(0, 256);

    return QString::fromLatin1(output, length);
}

QString randStrNotIn(int length, const QSet<QString> set, bool numChars, bool whiteSpaces)
{
    if (length == 0)
        return "";

    QString outStr;
    do
    {
        outStr = randStr(length, numChars, whiteSpaces);
    }
    while (set.contains(outStr));

    return outStr;
}


Range::Range() :
    from(0), to(0)
{
}

Range::Range(qint64 from, qint64 to)
    :from(from), to(to)
{
    fromValid = true;
    toValid = true;
}

void Range::setFrom(qint64 from)
{
    this->from = from;
    fromValid = true;
}

void Range::setTo(qint64 to)
{
    this->to = to;
    toValid = true;
}

qint64 Range::getFrom() const
{
    return from;
}

qint64 Range::getTo() const
{
    return to;
}

bool Range::isValid() const
{
    return fromValid && toValid && from <= to;
}

bool Range::contains(qint64 position) const
{
    return position >= from && position <= to;
}

bool Range::overlaps(const Range& other) const
{
    return overlaps(other.from, other.to);
}

bool Range::overlaps(qint64 from, qint64 to) const
{
    return (this->from >= from && this->from <= to) || (this->to >= from && this->to <= to);
}

Range Range::common(const Range& other) const
{
    return common(other.from, other.to);
}

Range Range::common(qint64 from, qint64 to) const
{
    if (!isValid() || from > to)
        return Range();

    if (this->from >= from)
    {
        if (this->from > to)
            return Range();

        if (this->to < to)
            return Range(this->from, this->to);
        else
            return Range(this->from, to);
    }
    else
    {
        if (from > this->to)
            return Range();

        if (to < this->to)
            return Range(from, to);
        else
            return Range(from, this->to);
    }
}

qint64 Range::length() const
{
    return to - from + 1;
}

QString generateUniqueName(const QString& baseName, const QStringList& existingNames, Qt::CaseSensitivity cs)
{
    QString name = baseName;
    int i = 0;
    while (existingNames.contains(name, cs))
        name = baseName + QString::number(i++);

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

QStringList tokenizeArgs(const QString& str)
{
    QStringList results;
    QString token;
    bool quote = false;
    bool escape = false;
    QChar c;
    for (int i = 0; i < str.length(); i++)
    {
        c = str[i];
        if (escape)
        {
            token += c;
        }
        else if (c == '\\')
        {
            escape = true;
        }
        else if (quote)
        {
            if (c == '"')
            {
                results << token;
                token.clear();
                quote = false;
            }
            else
            {
                token += c;
            }
        }
        else if (c == '"')
        {
            if (token.length() > 0)
                token += c;
            else
                quote = true;
        }
        else if (c.isSpace())
        {
            if (token.length() > 0)
            {
                results << token;
                token.clear();
            }
        }
        else
        {
            token += c;
        }
    }

    if (token.length() > 0)
        results << token;

    return results;
}

QStringList prefixEach(const QString& prefix, const QStringList& list)
{
    QStringList result;
    for (const QString& item : list)
        result << (prefix + item);

    return result;
}

QByteArray hashToBytes(const QHash<QString,QVariant>& hash)
{
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    stream << QVariant(hash);
    return bytes;
}

QHash<QString,QVariant> bytesToHash(const QByteArray& bytes)
{
    if (bytes.isNull())
        return QHash<QString,QVariant>();

    QVariant deserializedValue;
    QDataStream stream(bytes);
    stream >> deserializedValue;
    return deserializedValue.toHash();
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

QString center(const QString& str, int length, const QChar& fillChar)
{
    if (str.length() >= length)
        return str;

    QString result = str;
    QString fillLeft = QString(fillChar).repeated((length - str.length()) / 2);
    QString fillRight = fillLeft;
    if ((fillLeft.length() + fillRight.length() + str.length()) < length)
        fillLeft += fillChar;

    return result.prepend(fillLeft).append(fillRight);
}

const QString& longest(const QStringList& strList)
{
    int max = 0;
    qsizetype maxIndex = -1;
    for (qsizetype i = 0; i < strList.size(); i++)
    {
        int size = strList.at(i).size();
        if (size > max)
        {
            maxIndex = i;
            max = size;
        }
    }
    return strList.at(maxIndex);
}

const QString& shortest(const QStringList& strList)
{
    int max = INT_MAX;
    qsizetype maxIndex = -1;
    for (qsizetype i = 0; i < strList.size(); i++)
    {
        int size = strList.at(i).size();
        if (size < max)
        {
            maxIndex = i;
            max = size;
        }
    }
    return strList.at(maxIndex);
}

QString longestCommonPart(const QStringList& strList)
{
   if (strList.size() == 0)
       return QString();

   QString common;
   QString first = strList.first();
   for (int i = 0; i < first.length(); i++)
   {
       common += first[i];
       for (const QString& str : strList)
       {
           if (!str.startsWith(common))
               return common.left(i);
       }
   }
   return common;
}

QStringList applyMargin(const QString& str, int margin)
{
    QStringList lines;
    QString line;
    for (QString word : str.split(" "))
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

    int mmmBase = qRound(F * 86400000.0);
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

QStringList common(const QStringList& list1, const QStringList& list2, Qt::CaseSensitivity cs)
{
    QStringList newList;
    for (const QString& str : list1)
    {
        if (list2.contains(str, cs))
            newList << str;
    }
    return newList;
}

QStringList textCodecNames()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
    QStringList codecs = QStringConverter::availableCodecs();
#else
    QStringList codecs;
    for (QStringConverter::Encoding enc : {
         QStringConverter::Utf8,
         QStringConverter::Utf16,
         QStringConverter::Utf16LE,
         QStringConverter::Utf16BE,
         QStringConverter::Utf32,
         QStringConverter::Utf32LE,
         QStringConverter::Utf32BE,
         QStringConverter::Latin1,
         QStringConverter::System
    })
    {
        codecs << QStringConverter::nameForEncoding(enc);
    }
#endif
    QSet<QString> codecSet = QSet<QString>(codecs.begin(), codecs.end());
    QStringList names = QStringList(codecSet.begin(), codecSet.end());
    sSort(names);
    return names;
}

QStringEncoder* textEncoderForName(const QString& name)
{
    return new QStringEncoder(name.toLatin1());
}

QStringDecoder* textDecoderForName(const QString& name)
{
    return new QStringDecoder(name.toLatin1());
}

QStringEncoder* defaultTextEncoder()
{
    return new QStringEncoder();
}

QStringDecoder* defaultTextDecoder()
{
    return new QStringDecoder();
}

QString defaultCodecName()
{
    return QStringConverter::nameForEncoding(QStringConverter::System);
}

QStringList splitByLines(const QString& str)
{
    return str.split(QRegularExpression("\r?\n"));
}

QString joinLines(const QStringList& lines)
{
#ifdef Q_OS_WIN
    static_char* newLine = "\r\n";
#else
    static_char* newLine = "\n";
#endif
    return lines.join(newLine);
}

int sum(const QList<int>& integers)
{
    int res = 0;
    for (int i : integers)
        res += i;

    return res;
}

DistributionType getDistributionType()
{
#if defined(Q_OS_OSX)
    return DistributionType::OSX_BUNDLE;
#elif defined(PORTABLE_CONFIG)
    return DistributionType::PORTABLE;
#else
    return DistributionType::OS_MANAGED;
#endif
}

bool validateEmail(const QString& email)
{
    static const QRegularExpression re("^[a-zA-Z0-9_\\.-]+@[a-zA-Z0-9-]+.[a-zA-Z0-9-\\.]+$");
    return re.match(email).hasMatch();
}

bool isHex(const QString& str)
{
    bool ok;
    str.toLongLong(&ok, 16);
    return ok;
}

bool isHex(const QChar& c)
{
    return isHex(c.toLatin1());
}

bool isHex(const char c)
{
    switch (c)
    {
        case 'a':
        case 'A':
        case 'b':
        case 'B':
        case 'c':
        case 'C':
        case 'd':
        case 'D':
        case 'e':
        case 'E':
        case 'f':
        case 'F':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            return true;
    }
    return false;
}

QString formatVersion(int version)
{
    int majorVer = version / 10000;
    int minorVer = version % 10000 / 100;
    int patchVer = version % 100;
    return QString::number(majorVer) + "." + QString::number(minorVer) + "." + QString::number(patchVer);
}

bool copyRecursively(const QString& src, const QString& dst)
{
    // Code taken from QtCreator:
    // https://qt.gitorious.org/qt-creator/qt-creator/source/1a37da73abb60ad06b7e33983ca51b266be5910e:src/app/main.cpp#L13-189
    QFileInfo srcFileInfo(src);
    if (srcFileInfo.isDir())
    {
        QDir targetDir(dst);
        targetDir.cdUp();
        if (!targetDir.mkdir(QFileInfo(dst).fileName()))
            return false;

        QDir sourceDir(src);
        QStringList fileNames = sourceDir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System);
        for (const QString &fileName : fileNames)
        {
            const QString newSrcFilePath = src + QLatin1Char('/') + fileName;
            const QString newTgtFilePath = dst + QLatin1Char('/') + fileName;
            if (!copyRecursively(newSrcFilePath, newTgtFilePath))
                return false;
        }
    }
    else if (srcFileInfo.isSymLink())
    {
        QString trg = QFile(src).symLinkTarget();
        QFile::link(trg, dst);
    }
    else
    {
        if (!QFile::copy(src, dst))
            return false;
    }
    return true;
}

bool renameBetweenPartitions(const QString& src, const QString& dst)
{
    if (QDir(dst).exists())
        return false;

    int res = copyRecursively(src, dst);
    if (res)
        QDir(src).removeRecursively();
    else
        QDir(dst).removeRecursively();

    return res;
}

bool isWritableRecursively(const QString& dir)
{
    QFileInfo fi(dir);
    if (!fi.isWritable())
        return false;

    if (fi.isDir())
    {
        QStringList fileNames = QDir(dir).entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System);
        for (const QString &fileName : fileNames)
        {
            if (!isWritableRecursively(dir + QLatin1Char('/') + fileName))
                return false;
        }
    }
    return true;
}

QString encryptRsa(const QString& input, const QString& modulus, const QString& exponent)
{
    std::string inputStdStr = input.toStdString();
    Key key = Key(BigInt(modulus.toStdString()), BigInt(exponent.toStdString()));
    std::string result = RSA::Encrypt(inputStdStr, key);
    return QString::fromStdString(result);
}

QString decryptRsa(const QString& input, const QString& modulus, const QString& exponent)
{
    std::string inputStdStr = input.toStdString();
    Key key = Key(BigInt(modulus.toStdString()), BigInt(exponent.toStdString()));
    std::string result = RSA::Decrypt(inputStdStr, key);
    return QString::fromStdString(result);
}

QStringList concat(const QList<QStringList>& list)
{
    QStringList result;
    for (const QStringList& item : list)
        result.append(item);

    return result;
}


QString doubleToString(const QVariant& val)
{
    QString str = val.toString();
    if (str.contains("e") || str.length() - (str.indexOf('.') + 1) > 14)
    {
        str = QString::number(val.toDouble(), 'f', 14).remove(QRegularExpression("0*$"));
        if (str.endsWith("."))
            str += "0";
    }
    else if (!str.contains('.'))
        str += ".0";

    return str;
}

void sortWithReferenceList(QList<QString>& listToSort, const QList<QString>& referenceList, Qt::CaseSensitivity cs)
{
    sSort(listToSort, [referenceList, cs](const QString& s1, const QString& s2) -> bool
    {
        int idx1 = indexOf(referenceList, s1, cs);
        int idx2 = indexOf(referenceList, s2, cs);
        if (idx1 == -1 || idx2 == -1)
        {
            if (idx1 == -1 && idx2 == -1)
                return false;

            return (idx1 == -1);
        }

        if (idx1 == idx2)
            return false;

        return (idx1 > idx2);
    });
}

QByteArray serializeToBytes(const QVariant& value)
{
    return serializeToBytes(value, QDataStream::Qt_5_15);
}

QByteArray serializeToBytes(const QVariant& value, QDataStream::Version version)
{
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    stream.setVersion(version);
    stream << value;
    return bytes;
}

QVariant deserializeFromBytes(const QByteArray& bytes)
{
    return deserializeFromBytes(bytes, QDataStream::Qt_5_15);
}

QVariant deserializeFromBytes(const QByteArray& bytes, QDataStream::Version version)
{
    if (bytes.isNull())
        return QVariant();

    QVariant deserializedValue;
    QDataStream stream(bytes);
    stream.setVersion(version);
    stream >> deserializedValue;
    return deserializedValue;
}

QString readFileContents(const QString& path, QString* err)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        if (err)
            *err = QObject::tr("Could not open file '%1' for reading: %2").arg(path).arg(file.errorString());

        return QString();
    }

    QTextStream stream(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    stream.setCodec("UTF-8");
#else
    // UTF-8 is the default QTextStream encoding in Qt 6
#endif
    QString contents = stream.readAll();
    file.close();

    return contents;
}


TYPE_OF_QHASH qHash(const QVariant& var)
{
    if (!var.isValid() || var.isNull())
        return -1;

    switch (var.userType())
    {
        case QMetaType::Int:
            return qHash(var.toInt());
        case QMetaType::UInt:
            return qHash(var.toUInt());
        case QMetaType::Bool:
            return qHash(var.toUInt());
        case QMetaType::Double:
            return qHash(var.toUInt());
        case QMetaType::LongLong:
            return qHash(var.toLongLong());
        case QMetaType::ULongLong:
            return qHash(var.toULongLong());
        case QMetaType::QString:
            return qHash(var.toString());
        case QMetaType::QChar:
            return qHash(var.toChar());
        case QMetaType::QStringList:
            return qHash(var.toString());
        case QMetaType::QByteArray:
            return qHash(var.toByteArray());
        case QMetaType::QDate:
        case QMetaType::QTime:
        case QMetaType::QDateTime:
        case QMetaType::QUrl:
        case QMetaType::QLocale:
        case QMetaType::QRegularExpression:
            return qHash(var.toString());
        case QMetaType::QVariantHash:
            return qHash(var.toHash());
        case QMetaType::QVariantMap:
            return qHash(var.toMap());
        case QMetaType::QVariantList:
            return qHash(var.toList());
        case QMetaType::QBitArray:
            return qHash(var.toBitArray());
        case QMetaType::QSize:
        case QMetaType::QSizeF:
        case QMetaType::QRect:
        case QMetaType::QLineF:
        case QMetaType::QLine:
        case QMetaType::QRectF:
        case QMetaType::QPoint:
        case QMetaType::QPointF:
            // not supported yet
            break;
        case QMetaType::User:
        case QMetaType::UnknownType:
        default:
            return -3;
    }

    // could not generate a hash for the given variant
    return -2;
}

QString indentMultiline(const QString& str)
{
    QStringList lines = str.split("\n");
    for (QString& line : lines)
        line = line.prepend("    ");

    return lines.join("\n");
}

QString toNativePath(const QString& path)
{
    return QDir::toNativeSeparators(path);
}

QStringList sharedLibFileFilters()
{
    static QStringList filters({
#ifdef Q_OS_WIN
        "*.dll"
#elif defined Q_OS_MACOS
        "*.dylib"
#elif defined Q_OS_LINUX || Q_OS_BSD
        "*.so"
#endif
    });
    return filters;
}

void runInThread(std::function<void()> func)
{
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    class FunctionRunnable : public QRunnable
    {
        std::function<void()> func;
    public:
        FunctionRunnable(std::function<void()> func) : func(std::move(func)) {}
        void run() override
        {
            func();
        }
    };
    QThreadPool::globalInstance()->start(new FunctionRunnable(func));
#else
    QThreadPool::globalInstance()->start(func);
#endif
}

QStringConverter::Encoding textEncodingForName(const QString& name)
{
    auto encoding = QStringConverter::encodingForName(name.toLatin1().constData());
    return encoding ? *encoding : QStringConverter::System;
}
