#ifndef UTILS_H
#define UTILS_H

#include "coreSQLiteStudio_global.h"
#include <functional>
#include <QList>
#include <QMutableListIterator>
#include <QSet>
#include <QChar>
#include <QStringList>
#include <QFileInfo>
#include <functional>
#include <QVariant>
#include <QDataStream>

class QTextCodec;

API_EXPORT void initUtils();

class API_EXPORT Range
{
    public:
        Range();
        Range(qint64 from, qint64 to);

        void setFrom(qint64 from);
        void setTo(qint64 to);
        qint64 getFrom() const;
        qint64 getTo() const;
        bool isValid() const;
        bool contains(qint64 position) const;
        bool overlaps(const Range& other) const;
        bool overlaps(qint64 from, qint64 to) const;
        Range common(const Range& other) const;
        Range common(qint64 from, qint64 to) const;
        qint64 length() const;

    private:
        qint64 from;
        qint64 to;
        bool fromValid = false;
        bool toValid = false;
};

API_EXPORT bool isXDigit(const QChar& c);

/**
 * @brief Get character from string.
 * @param str String to get char from.
 * @param pos Character position.
 * @return Requested character or null character.
 *
 * This is safe getter for a character of the string,
 * thish returns null if pos index is out of range.
 */
QChar API_EXPORT charAt(const QString& str, int pos);

API_EXPORT int rand(int min = 0, int max = RAND_MAX);
API_EXPORT QString randStr(int length, bool numChars = true, bool whiteSpaces = false);
API_EXPORT QString randStr(int length, const QString& charCollection);
API_EXPORT QString randBinStr(int length);
API_EXPORT QString randStrNotIn(int length, const QSet<QString> set, bool numChars = true, bool whiteSpaces = false);
API_EXPORT QString generateUniqueName(const QString& prefix, const QStringList& existingNames, Qt::CaseSensitivity cs = Qt::CaseSensitive);
API_EXPORT bool isNumeric(const QVariant& value);
API_EXPORT QString rStrip(const QString& str);
API_EXPORT QStringList tokenizeArgs(const QString& str);
API_EXPORT QStringList prefixEach(const QString& prefix, const QStringList& list);
API_EXPORT QByteArray hashToBytes(const QHash<QString,QVariant>& hash);
API_EXPORT QHash<QString,QVariant> bytesToHash(const QByteArray& bytes);
API_EXPORT QString indentMultiline(const QString& str);
/**
  * @brief indexOf Extension to QStringList::indexOf().
  *
  * This method does pretty much the same as QStringList::indexOf(), except it supports
  * case sensitivity flag, unlike the original method.
  */
API_EXPORT int indexOf(const QStringList& list, const QString& value, int from = 0, Qt::CaseSensitivity cs = Qt::CaseSensitive);
API_EXPORT int indexOf(const QStringList& list, const QString& value, Qt::CaseSensitivity cs = Qt::CaseSensitive);

template <class T>
int indexOf(const QList<T>& list, std::function<bool(const T&)> predicate)
{
    int i = 0;
    for (const T& item : list)
    {
        if (predicate(item))
            return i;

        ++i;
    }
    return -1;
}

template <class T>
T* findFirst(const QList<T*>& list, std::function<bool(T*)> predicate)
{
    for (T* item : list)
    {
        if (predicate(item))
            return item;
    }
    return nullptr;
}

/**
 * @brief Returns only those elements from the list, which passed the filter.
 * @tparam T type for which the filter will be applied for. It should match the type in the list and in the function argument.
 * @param list List to filter elements from.
 * @param filterFunction Function that accepts elements from the list and returns true for elements that should be returned by the filter.
 * @return List of elements that passed custom function validation.
 */
template <class T>
QList<T> filter(const QList<T>& list, std::function<bool(const T& value)> filterFunction)
{
    QList<T> results;
    for (const T& value : list)
    {
        if (filterFunction(value))
            results << value;
    }
    return results;
}

template <class T>
bool contains(const QList<T>& list, std::function<bool(const T& value)> testFunction)
{
    for (const T& value : list)
    {
        if (testFunction(value))
            return true;
    }
    return false;
}

template <class T>
QList<T> concat(const QList<QList<T>>& list)
{
    QList<T> result;
    for (const QList<T>& item : list)
        result.append(item);

    return result;
}

template <class T, typename R = QList<T>>
R concat(const QList<QSet<T>>& list)
{
    R result;
    for (const QSet<T>& itemSet : list)
        for (const T& subitem : itemSet)
            result << subitem;

    return result;
}

template <class T>
QSet<T> concatSet(const QList<QSet<T>>& list)
{
    return concat<T, QSet<T>>(list);
}

template <class T>
QSet<T> concat(const QSet<QSet<T>>& list)
{
    QSet<T> result;
    for (const QSet<T>& item : list)
        result.unite(item);

    return result;
}

API_EXPORT QStringList concat(const QList<QStringList>& list);

/**
 * @brief Appends or prepends characters to the string to make it of specified length.
 * @param str Input string to work with.
 * @param length Desired length of output string.
 * @param fillChar Character to use to fill missing part of string.
 * @param String which is at least \p length characters long, using \p str as an initial value.
 *
 * It appends or prepends as many \p fillChar characters to the \p str, so the \p str becomes \p length characters long.
 * In case the \p str is already \p length characters long, or even longer, then the original string is returned.
 *
 * If \p length is positive value, characters are appended to string. It it's negative, then values are prepended to the string,
 * using an absolute value of the \p length for calculating output length.
 */
API_EXPORT QString pad(const QString& str, int length, const QChar& fillChar);

API_EXPORT QString center(const QString& str, int length, const QChar& fillChar);

/**
 * @brief Picks the longest string from the list.
 * @param strList List to pick from.
 * @return Longest value from the list, or empty string if the \p strList was empty as well.
 *
 * If there are many values with the same, longest length, then the first one is picked.
 */
API_EXPORT const QString& longest(const QStringList& strList);

/**
 * @brief Picks the shortest string from the list.
 * @param strList List to pick from.
 * @return Shortest value from the list, or empty string if the \p strList was empty as well.
 *
 * If there are many values with the same, shortest length, then the first one is picked.
 */
API_EXPORT const QString& shortest(const QStringList& strList);

/**
 * @brief Finds the longest common part of all strings.
 * @param strList List to compare strings from.
 * @return Longest common string (looking from the begining of each string) from the list.
 */
API_EXPORT QString longestCommonPart(const QStringList& strList);

/**
 * @brief Applies margin of given number of characters to the string, splitting it into lines.
 * @param str String to apply the margin to.
 * @param margin Number of characters allows in single line.
 * @return List of lines produced by applying the margin.
 *
 * If \p str is longer than \p margin number of characters, this method splits \p str into several lines
 * in order to respect the \p margin. White spaces are taken as points of splitting, but if there is
 * a single word longer than \p margin, then this word gets splitted.
 */
API_EXPORT QStringList applyMargin(const QString& str, int margin);

/**
 * @brief toGregorian Converts Julian Date to Gregorian Date.
 * @param julianDateTime Floating point representing Julian Day and Julian time.
 * @return Gregorian date.
 *
 * Converts Julian Calendar date into Gregorian Calendar date.
 * See Wikipedia for details.
 */
API_EXPORT QDateTime toGregorian(double julianDateTime);

/**
 * @brief toJulian Converts Gregorian Date to Julian Date.
 * @param gregDateTime Gregorian calendar date and time.
 * @return Julian calendar date and time.
 *
 * Converts the usually used Gregorian date and time into Julian Date format,
 * which is floating point, where integral part is the Julian Day and fraction part is time of the day.
 */
API_EXPORT double toJulian(const QDateTime& gregDateTime);

/**
 * @brief toJulian Converts Gregorian Date to Julian Date.
 * @overload
 */
API_EXPORT double toJulian(int year, int month, int day, int hour, int minute, int second, int msec);

API_EXPORT QString formatFileSize(quint64 size);

API_EXPORT QString formatTimePeriod(int msecs);

API_EXPORT QStringList common(const QStringList& list1, const QStringList& list2, Qt::CaseSensitivity cs = Qt::CaseSensitive);

API_EXPORT QStringList textCodecNames();
API_EXPORT QString defaultCodecName();
API_EXPORT QTextCodec* defaultCodec();
API_EXPORT QTextCodec* codecForName(const QString& name);
API_EXPORT QStringList splitByLines(const QString& str);
API_EXPORT QString joinLines(const QStringList& lines);
API_EXPORT QStringList sharedLibFileFilters();
API_EXPORT int sum(const QList<int>& integers);
API_EXPORT QString getOsString();
API_EXPORT bool validateEmail(const QString& email);
API_EXPORT bool isHex(const QString& str);
API_EXPORT bool isHex(const QChar& c);
API_EXPORT bool isHex(const char c);
API_EXPORT QString formatVersion(int version);
API_EXPORT bool copyRecursively(const QString& src, const QString& dst);
API_EXPORT bool renameBetweenPartitions(const QString& src, const QString& dst);
API_EXPORT bool isWritableRecursively(const QString& dir);
API_EXPORT QString encryptRsa(const QString& input, const QString& modulus, const QString& exponent);
API_EXPORT QString decryptRsa(const QString& input, const QString& modulus, const QString& exponent);
API_EXPORT QString doubleToString(const QVariant& val);

/**
 * @brief Sorts string list using reference list for ordering.
 * @param listToSort This list will be sorted.
 * @param referenceList Should contain all elements from list to sort - it tells the order.
 * @param cs Case sensitivity of the string comparision.
 *
 * Sorts \p listToSort using \p referenceList as a reference of what is the order.
 * If any element from \p listToSort is not on the list of \p referenceList, then the element will be placed at the end.
 */
API_EXPORT void sortWithReferenceList(QList<QString>& listToSort, const QList<QString>& referenceList, Qt::CaseSensitivity cs = Qt::CaseSensitive);

enum class DistributionType
{
    PORTABLE,
    OSX_BUNDLE,
    OS_MANAGED
};

API_EXPORT DistributionType getDistributionType();

template <class T>
QList<T> reverse(const QList<T>& list)
{
    QList<T> result;
    for (const T& el : list)
        result.prepend(el);

    return result;
}

template <class S, class T>
QList<T> map(const QList<S>& list, std::function<T(S)> transformer)
{
    QList<T> result;
    for (const S& el : list)
        result << transformer(el);

    return result;
}

template <class S, class T>
QHash<S, T> toHash(const QList<S>& list, std::function<T(S)> transformer)
{
    QHash<S, T> result;
    for (const S& el : list)
        result[el] = transformer(el);

    return result;
}

template <class S, class T>
QSet<T> map(const QSet<S>& set, std::function<T(S)> transformer)
{
    QSet<T> result;
    for (const S& el : set)
        result << transformer(el);

    return result;
}

template <class T>
void removeDuplicates(QList<T>& list)
{
    QSet<T> set;
    QMutableListIterator<T> i(list);
    while (i.hasNext())
    {
        i.next();
        if (set.contains(i.value()))
            i.remove();
        else
            set << i.value();
    }
}

API_EXPORT uint qHash(const QVariant& var);

API_EXPORT QByteArray serializeToBytes(const QVariant& value);

API_EXPORT QVariant deserializeFromBytes(const QByteArray& bytes);

API_EXPORT QString readFileContents(const QString& path, QString* err);

API_EXPORT QString toNativePath(const QString& path);

Q_DECLARE_METATYPE(QList<int>)

#endif // UTILS_H
