#include "sqlresultsrow.h"

SqlResultsRow::SqlResultsRow()
{
}

SqlResultsRow::~SqlResultsRow()
{
}

const QVariant SqlResultsRow::value(const QString &key) const
{
    return valuesMap[key];
}

const QHash<QString, QVariant> &SqlResultsRow::valueMap() const
{
    return valuesMap;
}

const QList<QVariant>& SqlResultsRow::valueList() const
{
    return values;
}

const QVariant SqlResultsRow::value(int idx) const
{
    if (idx < 0 || idx >= values.size())
        return QVariant();

    return values[idx];
}

bool SqlResultsRow::contains(const QString &key) const
{
    return valuesMap.contains(key);
}

bool SqlResultsRow::contains(int idx) const
{
    return idx >= 0 && idx < values.size();
}
