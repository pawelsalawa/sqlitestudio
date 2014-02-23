#include "sqlresultsrowqt.h"

void SqlResultsRowQt::setValue(const QString &column, const QVariant &value)
{
    valuesMap.insert(column, value);
    values << value;
}

