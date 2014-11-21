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
