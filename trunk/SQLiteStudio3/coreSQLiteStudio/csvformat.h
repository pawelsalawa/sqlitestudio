#ifndef CSVFORMAT_H
#define CSVFORMAT_H

#include "coreSQLiteStudio_global.h"
#include <QString>
#include <QStringList>

struct API_EXPORT CsvFormat
{
    CsvFormat();
    CsvFormat(const QString& columnSeparator, const QString& rowSeparator);
    CsvFormat(const QStringList& columnSeparators, const QStringList& rowSeparators);
    CsvFormat(const QString& columnSeparator, const QString& rowSeparator, bool strictRowSeparator, bool strictColumnSeparator);

    QString columnSeparator;
    QString rowSeparator;
    QStringList columnSeparators;
    QStringList rowSeparators;
    bool strictColumnSeparator = false;
    bool strictRowSeparator = false;
    bool multipleRowSeparators = false;
    bool multipleColumnSeparators = false;

    static const CsvFormat DEFAULT;
};

#endif // CSVFORMAT_H
