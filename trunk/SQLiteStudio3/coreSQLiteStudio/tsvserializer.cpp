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
    QStringList tokens;
    QStringList parts = data.split(columnSeparator);
    for (const QString& part : parts)
    {
        if (!part.contains(rowSeparator))
        {
            cells << part;
            continue;
        }

        tokens = tokenizeStrWithRowSeparator(part);
        for (const QString& token : tokens)
        {
            if (token != rowSeparator)
            {
                cells << token;
                continue;
            }

            rows << cells;
            cells.clear();
        }
    }

    if ((cells.size() > 0 && !cells.first().isEmpty()) || cells.size() > 1)
        rows << cells;

    return rows;
}

QStringList TsvSerializer::tokenizeStrWithRowSeparator(const QString& data)
{
    QStringList tokens;
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

            field += c;
        }
        else if (quotes && c == '"' )
        {
            field += c;
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
        else if (!quotes && c == rowSeparator)
        {
            tokens << flushToken(field);
            field.clear();
            tokens << QString(c);
        }
        else
        {
            field += c;
        }
        pos++;
    }

    if (field.size() > 0)
        tokens << flushToken(field);

    return tokens;
}

QString TsvSerializer::flushToken(const QString& token)
{
    if (!token.startsWith('"') || !token.contains(rowSeparator))
        return token;

    int decr = 1;
    if (token.endsWith('"'))
        decr++;

    return token.mid(1, token.length() - decr).replace("\"\"", "\"");
}
