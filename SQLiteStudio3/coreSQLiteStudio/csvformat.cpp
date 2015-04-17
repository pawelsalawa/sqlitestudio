#include "csvformat.h"
#include <QtGlobal>

const CsvFormat CsvFormat::DEFAULT = {",", "\n"};

CsvFormat::CsvFormat()
{
}

CsvFormat::CsvFormat(const QString& columnSeparator, const QString& rowSeparator) :
    columnSeparator(columnSeparator), rowSeparator(rowSeparator)
{
}

CsvFormat::CsvFormat(const QString& columnSeparator, const QString& rowSeparator, bool strictRowSeparator, bool strictColumnSeparator) :
    columnSeparator(columnSeparator), rowSeparator(rowSeparator), strictColumnSeparator(strictColumnSeparator), strictRowSeparator(strictRowSeparator)
{
}
