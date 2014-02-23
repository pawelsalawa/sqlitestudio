#ifndef CSVFORMAT_H
#define CSVFORMAT_H

#include "coreSQLiteStudio_global.h"
#include <QString>

struct API_EXPORT CsvFormat
{
    CsvFormat();
    CsvFormat(const QString& columnSeparator, const QString& rowSeparator);

    QString columnSeparator;
    QString rowSeparator;

    static const CsvFormat DEFAULT;
    static const CsvFormat CLIPBOARD;
};

#endif // CSVFORMAT_H
