#ifndef UTILS_H
#define UTILS_H

#include "coreSQLiteStudio_global.h"
#include <QList>
#include <QMutableListIterator>
#include <QSet>
#include <QChar>
#include <QStringList>

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
API_EXPORT QString randStr(int length, bool numChars = true);
API_EXPORT QString randBinStr(int length);
API_EXPORT QString randStrNotIn(int length, const QSet<QString> set, bool numChars = true);
API_EXPORT QString generateUniqueName(const QString& prefix, const QStringList& existingNames);
API_EXPORT bool isNumeric(const QVariant& value);
API_EXPORT QString rStrip(const QString& str);
API_EXPORT QStringList tokenizeArgs(const QString& str);
API_EXPORT QStringList prefixEach(const QString& prefix, const QStringList& list);
/**
  * @brief indexOf Extension to QStringList::indexOf().
  *
  * This method does pretty much the same as QStringList::indexOf(), except it supports
  * case sensitivity flag, unlike the original method.
  */
API_EXPORT int indexOf(const QStringList& list, const QString& value, int from = 0, Qt::CaseSensitivity cs = Qt::CaseSensitive);
API_EXPORT int indexOf(const QStringList& list, const QString& value, Qt::CaseSensitivity cs = Qt::CaseSensitive);

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
API_EXPORT QString longest(const QStringList& strList);

/**
 * @brief Picks the shortest string from the list.
 * @param strList List to pick from.
 * @return Shortest value from the list, or empty string if the \p strList was empty as well.
 *
 * If there are many values with the same, shortest length, then the first one is picked.
 */
API_EXPORT QString shortest(const QStringList& strList);

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

#endif // UTILS_H
