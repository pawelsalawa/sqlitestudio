#ifndef CSVFORMAT_H
#define CSVFORMAT_H

#include "coreSQLiteStudio_global.h"
#include <QString>

struct API_EXPORT CsvFormat
{
    CsvFormat();
    CsvFormat(const QString& columnSeparator, const QString& rowSeparator);
    CsvFormat(const QString& columnSeparator, const QString& rowSeparator, bool strictRowSeparator, bool strictColumnSeparator);

    QString columnSeparator;
    QString rowSeparator;
    bool strictColumnSeparator = false;
    bool strictRowSeparator = false;

    static const CsvFormat DEFAULT;
};

#endif // CSVFORMAT_H
