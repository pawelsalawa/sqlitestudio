#include "sqlresultsrow.h"

SqlResultsRow::SqlResultsRow()
{
}

SqlResultsRow::~SqlResultsRow()
{
}

QVariant &SqlResultsRow::value(const QString &key)
{
    return valuesMap[key];
}

QHash<QString, QVariant> &SqlResultsRow::valueMap()
{
    return valuesMap;
}

QList<QVariant> SqlResultsRow::valueList()
{
    return values;
}

QVariant SqlResultsRow::value(int idx)
{
    return values[idx];
}

bool SqlResultsRow::contains(const QString &key)
{
    return valuesMap.contains(key);
}

bool SqlResultsRow::contains(int idx)
{
    return idx >= 0 && idx < values.size();
}
