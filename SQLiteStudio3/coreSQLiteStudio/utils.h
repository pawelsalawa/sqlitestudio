#ifndef UTILS_H
#define UTILS_H

#include "coreSQLiteStudio_global.h"
#include <QList>
#include <QMutableListIterator>
#include <QSet>
#include <QChar>
#include <QStringList>

API_EXPORT void initUtils();

struct API_EXPORT Range
{
    Range(qint64 from, qint64 to);

    bool contains(qint64 position);

    qint64 from;
    qint64 to;
};

API_EXPORT bool isXDigit(const QChar& c);

/**
 * @brief Get character from string.
 * @param str String to get char from.
 * @param pos Character position.
 * @return Requested character or null character.
 * This is safe getter for a character of the string,
 * thish returns null if pos index is out of range.
 */
QChar API_EXPORT charAt(const QString& str, int pos);

API_EXPORT int rand(int min = 0, int max = RAND_MAX);
API_EXPORT QString randStr(int length, bool numChars = true);
API_EXPORT QString randBinStr(int length);
API_EXPORT QString randStrNotIn(int length, const QSet<QString> set);
API_EXPORT QString generateUniqueName(const QString& prefix, const QStringList& existingNames);
API_EXPORT bool isNumeric(const QVariant& value);
API_EXPORT QString rStrip(const QString& str);

/**
  * @brief indexOf Extension to QStringList::indexOf().
  * This method does pretty much the same as QStringList::indexOf(), except it supports
  * case sensitivity flag, unlike the original method.
  */
API_EXPORT int indexOf(const QStringList& list, const QString& value, int from = 0, Qt::CaseSensitivity cs = Qt::CaseSensitive);
API_EXPORT int indexOf(const QStringList& list, const QString& value, Qt::CaseSensitivity cs = Qt::CaseSensitive);

/**
 * @brief toGregorian Converts Julian Date to Gregorian Date.
 * @param julianDateTime Floating point representing Julian Day and Julian time.
 * @return Gregorian date.
 * Converts Julian Calendar date into Gregorian Calendar date.
 * See Wikipedia for details.
 */
API_EXPORT QDateTime toGregorian(double julianDateTime);

/**
 * @brief toJulian Converts Gregorian Date to Julian Date.
 * @param gregDateTime Gregorian calendar date and time.
 * @return Julian calendar date and time.
 * Converts the usually used Gregorian date and time into Julian Date format,
 * which is floating point, where integral part is the Julian Day and fraction part is time of the day.
 */
API_EXPORT double toJulian(const QDateTime& gregDateTime);

/**
 * @brief toJulian Converts Gregorian Date to Julian Date.
 * @overload double toJulian(const QDateTime& gregDateTime)
 */
API_EXPORT double toJulian(int year, int month, int day, int hour, int minute, int second, int msec);

API_EXPORT QString formatFileSize(quint64 size);

API_EXPORT QString formatTimePeriod(int msecs);

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
