#include "csvserializer.h"
#include <QStringList>
#include <QList>
#include <QDebug>
#include <QTime>
#include <QIODevice>

template <class C>
bool isCsvSeparator(QList<C>& ahead, const C& theChar, const QStringList& separators)
{
    for (const QString& sep : separators)
        if (isCsvSeparator(ahead, theChar, sep))
            return true;

    return false;
}

template <class C>
bool isCsvSeparator(QList<C>& ahead, const C& theChar, const QString& singleSeparator)
{
    if (singleSeparator[0] != theChar)
        return false;

    typename QList<C>::const_iterator aheadIter = ahead.begin();
    int singleSeparatorSize = singleSeparator.size();
    int i = 1;
    while (aheadIter != ahead.end() && i < singleSeparatorSize)
    {
        if (singleSeparator[i++] != *aheadIter++)
            return false;
    }

    if (i < singleSeparatorSize)
        return false;

    for (int i = 1, total = singleSeparator.size(); i < total; ++i)
        ahead.removeFirst();

    return true;
}

template <class C>
bool isCsvColumnSeparator(QList<C>& ahead, const C& theChar, const CsvFormat& format)
{
    if (!format.strictColumnSeparator)
        return format.columnSeparator.contains(theChar);

    // Strict checking (characters in defined order make a separator)
    if (format.multipleColumnSeparators)
        return isCsvSeparator(ahead, theChar, format.columnSeparators);

    return isCsvSeparator(ahead, theChar, format.columnSeparator);
}

template <class C>
bool isCsvRowSeparator(QList<C>& ahead, const C& theChar, const CsvFormat& format)
{
    if (!format.strictRowSeparator)
        return format.rowSeparator.contains(theChar);

    // Strict checking (characters in defined order make a separator)
    if (format.multipleRowSeparators)
        return isCsvSeparator(ahead, theChar, format.rowSeparators);

    return isCsvSeparator(ahead, theChar, format.rowSeparator);
}

template <class C>
void readAhead(QTextStream& data, QList<C>& ahead, int desiredSize)
{
    C singleValue;
    while (!data.atEnd() && ahead.size() < desiredSize)
    {
        data >> singleValue;
        ahead << singleValue;
    }
}

template <class T, class C>
void typedDeserializeInternal(QTextStream& data, const CsvFormat& format, QList<T>* cells, QList<QList<T>>* rows)
{
    bool quotes = false;
    bool sepAsLast = false;
    int separatorMaxAhead = qMax(format.maxColumnSeparatorLength, format.maxRowSeparatorLength) - 1;
    T field = "";
    field.reserve(3);
    C theChar;
    QList<C> ahead;

    while (!data.atEnd() || !ahead.isEmpty())
    {
        if (!ahead.isEmpty())
            theChar = ahead.takeFirst();
        else
            data >> theChar;

        sepAsLast = false;
        if (format.quotationMark && !quotes && theChar == '"' )
        {
            quotes = true;
        }
        else if (quotes && theChar == '"' )
        {
            if (!data.atEnd())
            {
                readAhead(data, ahead, 1);
                if (ahead.isEmpty())
                {
                    field += theChar;
                }
                else if (ahead.first() == '"' )
                {
                   field += theChar;
                   ahead.removeFirst();
                }
                else
                {
                   quotes = false;
                }
            }
            else
            {
                if (field.length() == 0)
                    *cells << field;

                quotes = false;
            }
        }
        else if (!quotes)
        {
            readAhead(data, ahead, separatorMaxAhead);
            if (isCsvColumnSeparator(ahead, theChar, format))
            {
                *cells << field;
                field.truncate(0);
                sepAsLast = true;
            }
            else if (isCsvRowSeparator(ahead, theChar, format))
            {
                *cells << field;
                field.truncate(0);
                if (rows)
                {
                    *rows << *cells;
                    cells->clear();
                }
                else
                {
                    break;
                }
            }
            else
            {
                field += theChar;
            }
        }
        else
        {
            field += theChar;
        }
    }

    if (field.size() > 0 || sepAsLast)
        *cells << field;

    if (rows)
    {
        if (cells->size() > 0)
            *rows << *cells;
    }
    else if (!ahead.isEmpty())
    {
        data.seek(data.pos() - 1);
    }
}

template <class T, class C>
QList<QList<T>> typedDeserialize(QTextStream& data, const CsvFormat& format)
{
    QList<QList<T>> rows;
    QList<T> cells;
    typedDeserializeInternal<T, C>(data, format, &cells, &rows);
    return rows;
}

template <class T, class C>
QList<T> typedDeserializeOneEntry(QTextStream& data, const CsvFormat& format)
{
    QList<T> cells;
    typedDeserializeInternal<T, C>(data, format, &cells, nullptr);
    return cells;
}

QString CsvSerializer::serialize(const QList<QStringList>& data, const CsvFormat& format)
{
    QStringList outputRows;

    for (const QStringList& dataRow : data)
        outputRows << serialize(dataRow, format);

    return outputRows.join(format.rowSeparator);
}

QString CsvSerializer::serialize(const QStringList& data, const CsvFormat& format)
{
    QString value;
    bool hasQuote;
    QStringList outputCells;
    for (const QString& rowValue : data)
    {
        value = rowValue;

        hasQuote = value.contains("\"");
        if (hasQuote)
            value.replace("\"", "\"\"");

        if (hasQuote || value.contains(format.columnSeparator) || value.contains(format.rowSeparator))
            value = "\""+value+"\"";

        outputCells << value;
    }

    return outputCells.join(format.columnSeparator);
}

QStringList CsvSerializer::deserializeOneEntry(QTextStream& data, const CsvFormat& format)
{
    QList<QString> deserialized = typedDeserializeOneEntry<QString, QChar>(data, format);
    return QStringList(deserialized);
}

QList<QList<QByteArray>> CsvSerializer::deserialize(const QByteArray& data, const CsvFormat& format)
{
    QTextStream stream(data, QIODevice::ReadWrite);
    stream.setEncoding(QStringConverter::Latin1);
    return typedDeserialize<QByteArray,char>(stream, format);
}

QList<QStringList> CsvSerializer::deserialize(QTextStream& data, const CsvFormat& format)
{
    QList<QList<QString>> deserialized = typedDeserialize<QString, QChar>(data, format);

    QList<QStringList> finalList;
    for (const QList<QString>& resPart : deserialized)
        finalList << QStringList(resPart);

    return finalList;
}

QList<QStringList> CsvSerializer::deserialize(const QString& data, const CsvFormat& format)
{
    QString dataString = data;
    QTextStream stream(&dataString, QIODevice::ReadWrite);
    stream.setEncoding(QStringConverter::Latin1);
    return deserialize(stream, format);
}
