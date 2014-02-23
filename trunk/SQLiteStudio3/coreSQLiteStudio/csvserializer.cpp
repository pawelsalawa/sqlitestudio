#include "csvserializer.h"
#include <QStringList>

// TODO write unit tests for CsvSerializer

QString CsvSerializer::serialize(const QList<QStringList>& data, const CsvFormat& format)
{
    QStringList outputCells;
    QStringList outputRows;
    QString value;
    bool hasQuote;

    foreach (const QStringList& dataRow, data)
    {
        foreach (const QString& rowValue, dataRow)
        {
            value = rowValue;

            hasQuote = value.contains("\"");
            if (hasQuote)
                value.replace("\"", "\"\"");

            if (hasQuote || value.contains(format.columnSeparator) || value.contains(format.rowSeparator))
                value = "\""+value+"\"";

            outputCells << value;
        }

        outputRows << outputCells.join(format.columnSeparator);
        outputCells.clear();
    }

    return outputRows.join(format.rowSeparator);
}

QList<QStringList> CsvSerializer::deserialize(const QString& data, const CsvFormat& format)
{
    QList<QStringList> rows;
    QStringList cells;

    int pos = 0;
    int lgt = data.length();
    bool quotes = false;
    QString field = "";
    QChar c;

    while (pos < lgt)
    {
        c = data[pos];
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
        else if (!quotes && c == format.columnSeparator)
        {
            cells << field;
            field.clear();
        }
        else if (!quotes && c == format.rowSeparator)
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

    if (field.size() > 0)
        cells << field;

    if (cells.size() > 0)
        rows << cells;

    return rows;
}
