#include "csvformat.h"
#include <QtGlobal>

const CsvFormat CsvFormat::DEFAULT = {",", "\n"};
const CsvFormat CsvFormat::CLIPBOARD = {"\t", "\n"};

CsvFormat::CsvFormat()
{
}

CsvFormat::CsvFormat(const QString& columnSeparator, const QString& rowSeparator)
{
    this->columnSeparator = columnSeparator;
    this->rowSeparator = rowSeparator;
}
