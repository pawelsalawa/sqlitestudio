#include "csvformat.h"
#include <QtGlobal>

const CsvFormat CsvFormat::DEFAULT = {",", "\r\n"};
const CsvFormat CsvFormat::CLIPBOARD = {"\t", "\r\n"};

CsvFormat::CsvFormat()
{
}

CsvFormat::CsvFormat(const QString& columnSeparator, const QString& rowSeparator)
{
    this->columnSeparator = columnSeparator;
    this->rowSeparator = rowSeparator;
}
