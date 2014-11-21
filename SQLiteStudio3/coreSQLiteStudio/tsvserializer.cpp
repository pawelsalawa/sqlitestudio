#include "tsvserializer.h"

#ifdef Q_OS_MACX
QString TsvSerializer::rowSeparator = "\r";
#else
QString TsvSerializer::rowSeparator = "\n";
#endif
QString TsvSerializer::columnSeparator = "\t";

QString TsvSerializer::serialize(const QList<QStringList>& data)
{
    QStringList outputRows;

    foreach (const QStringList& dataRow, data)
        outputRows << serialize(dataRow);

    return outputRows.join(rowSeparator);
}

QString TsvSerializer::serialize(const QStringList& data)
{
    QString value;
    bool hasQuote;
    QStringList outputCells;
    foreach (const QString& rowValue, data)
    {
        value = rowValue;

        hasQuote = value.contains("\"");
        if (value.contains(columnSeparator) || value.contains(rowSeparator))
        {
            if (hasQuote)
                value.replace("\"", "\"\"");

            value = "\""+value+"\"";
        }

        outputCells << value;
    }

    return outputCells.join(columnSeparator);
}

QList<QStringList> TsvSerializer::deserialize(const QString& data)
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
            if (field.isEmpty())
                quotes = true;
            else
                field += c;
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
        else if (!quotes && c == columnSeparator)
        {
            cells << field;
            field.clear();
        }
        else if (!quotes && c == rowSeparator)
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
