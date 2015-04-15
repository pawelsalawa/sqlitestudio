#include "csvserializer.h"
#include <QStringList>

// TODO write unit tests for CsvSerializer

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

template <class T>
QList<QList<T>> typedDeserialize(const T& data, const CsvFormat& format)
{
    QList<QList<T>> rows;
    QList<T> cells;

    int pos = 0;
    int lgt = data.length();
    bool quotes = false;
    bool sepAsLast = false;
    T field = "";
    QChar c;

    while (pos < lgt)
    {
        c = data[pos];
        sepAsLast = false;
        if (!quotes && c == '"' )
        {
            quotes = true;
        }
        else if (quotes && c == '"' )
        {
            if (pos + 1 < data.length() && data[pos+1] == '"' )
            {
               field += c;
               pos++;
            }
            else
            {
               quotes = false;
            }
        }
        else if (!quotes && format.columnSeparator.contains(c))
        {
            cells << field;
            field.clear();
            sepAsLast = true;
        }
        else if (!quotes && format.rowSeparator.contains(c))
        {
            cells << field;
            rows << cells;
            cells.clear();
            field.clear();
        }
        else
        {
            field += c;
        }
        pos++;
    }

    if (field.size() > 0 || sepAsLast)
        cells << field;

    if (cells.size() > 0)
        rows << cells;

    return rows;
}

QList<QList<QByteArray> > CsvSerializer::deserialize(const QByteArray& data, const CsvFormat& format)
{
    return typedDeserialize<QByteArray>(data, format);
}

QList<QStringList> CsvSerializer::deserialize(const QString& data, const CsvFormat& format)
{
    QList<QList<QString>> deserialized = typedDeserialize<QString>(data, format);

    QList<QStringList> finalList;
    for (const QList<QString>& resPart : deserialized)
        finalList << QStringList(resPart);

    return finalList;
}


//QList<QStringList> CsvSerializer::deserialize(const QByteArray& data, const CsvFormat& format)
//{
//    QList<QStringList> rows;
//    QStringList cells;

//    int pos = 0;
//    int lgt = data.length();
//    bool quotes = false;
//    bool sepAsLast = false;
//    QString field = "";
//    QChar c;

//    while (pos < lgt)
//    {
//        c = data[pos];
//        sepAsLast = false;
//        if (!quotes && c == '"' )
//        {
//            quotes = true;
//        }
//        else if (quotes && c == '"' )
//        {
//            if (pos + 1 < data.length() && data[pos+1] == '"' )
//            {
//               field += c;
//               pos++;
//            }
//            else
//            {
//               quotes = false;
//            }
//        }
//        else if (!quotes && format.columnSeparator.contains(c))
//        {
//            cells << field;
//            field.clear();
//            sepAsLast = true;
//        }
//        else if (!quotes && format.rowSeparator.contains(c))
//        {
//            cells << field;
//            rows << cells;
//            cells.clear();
//            field.clear();
//        }
//        else
//        {
//            field += c;
//        }
//        pos++;
//    }

//    if (field.size() > 0 || sepAsLast)
//        cells << field;

//    if (cells.size() > 0)
//        rows << cells;

//    return rows;
//}
