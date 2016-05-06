#include "csvserializer.h"
#include <QStringList>

// TODO write unit tests for CsvSerializer

template <class C>
bool isCsvColumnSeparator(QTextStream& data, const C& theChar, const CsvFormat& format)
{
    if (!format.strictColumnSeparator)
        return format.columnSeparator.contains(theChar);

    // Strict checking (characters in defined order make a separator)
    qint64 origPos = data.pos();
    data.seek(origPos - 1);
    C nextChar;
    for (const QChar& c : format.columnSeparator)
    {
        data >> nextChar;
        if (c != nextChar)
        {
            data.seek(origPos);
            return false;
        }
    }

    return true;
}

template <class C>
bool isCsvRowSeparator(QTextStream& data, const C& theChar, const CsvFormat& format)
{
    if (!format.strictRowSeparator)
        return format.rowSeparator.contains(theChar);

    // Strict checking (characters in defined order make a separator)
    qint64 origPos = data.pos();
    data.seek(origPos - 1);
    C nextChar;
    for (const QChar& c : format.rowSeparator)
    {
        data >> nextChar;
        if (data.atEnd() || c != nextChar)
        {
            data.seek(origPos);
            return false;
        }
    }

    return true;
}

template <class T, class C>
QList<QList<T>> typedDeserialize(QTextStream& data, const CsvFormat& format, bool oneEntry)
{
    QList<QList<T>> rows;
    QList<T> cells;

    bool quotes = false;
    bool sepAsLast = false;
    T field = "";
    C c0;
    C c1;

    while (!data.atEnd())
    {
        data >> c0;
        sepAsLast = false;
        if (!quotes && c0 == '"' )
        {
            quotes = true;
        }
        else if (quotes && c0 == '"' )
        {
            if (!data.atEnd())
            {
                data >> c1;
                if (c1 == '"' )
                {
                   field += c0;
                }
                else
                {
                   quotes = false;
                   data.seek(data.pos() - 1);
                }
            }
            else
            {
                if (field.length() == 0)
                    cells << field;

                quotes = false;
            }
        }
        else if (!quotes)
        {
            if (isCsvColumnSeparator(data, c0, format))
            {
                cells << field;
                field = "";
                sepAsLast = true;
            }
            else if (isCsvRowSeparator(data, c0, format))
            {
                cells << field;
                rows << cells;
                cells.clear();
                field = "";
                if (oneEntry)
                    break;
            }
            else
            {
                field += c0;
            }
        }
        else
        {
            field += c0;
        }
    }

    if (field.size() > 0 || sepAsLast)
        cells << field;

    if (cells.size() > 0)
        rows << cells;

    return rows;
}

QString CsvSerializer::serialize(const QList<QStringList>& data, const CsvFormat& format)
{
    QStringList outputRows;

    foreach (const QStringList& dataRow, data)
        outputRows << serialize(dataRow, format);

    return outputRows.join(format.rowSeparator);
}

QString CsvSerializer::serialize(const QStringList& data, const CsvFormat& format)
{
    QString value;
    bool hasQuote;
    QStringList outputCells;
    foreach (const QString& rowValue, data)
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
    QList<QStringList> deserialized = CsvSerializer::deserialize(data, format, true);
    if (deserialized.size() > 0)
        return deserialized.first();

    return QStringList();
}

QList<QStringList> CsvSerializer::deserialize(QTextStream& data, const CsvFormat& format)
{
    return CsvSerializer::deserialize(data, format, false);
}

QList<QList<QByteArray>> CsvSerializer::deserialize(const QByteArray& data, const CsvFormat& format)
{
    QTextStream stream(data, QIODevice::ReadWrite);
    return typedDeserialize<QByteArray,char>(stream, format, false);
}

QList<QStringList> CsvSerializer::deserialize(QTextStream& data, const CsvFormat& format, bool oneEntry)
{
    QList<QList<QString>> deserialized = typedDeserialize<QString, QChar>(data, format, oneEntry);

    QList<QStringList> finalList;
    for (const QList<QString>& resPart : deserialized)
        finalList << QStringList(resPart);

    return finalList;
}

QList<QStringList> CsvSerializer::deserialize(const QString& data, const CsvFormat& format)
{
    QString dataString = data;
    QTextStream stream(&dataString, QIODevice::ReadWrite);
    return deserialize(stream, format, false);
}

