#include "sqlresultrowandroid.h"

SqlResultRowAndroid::SqlResultRowAndroid(const QVariantHash& resultMap, const QVariantList& resultList)
{
    valuesMap = resultMap;
    values = resultList;
}

SqlResultRowAndroid::~SqlResultRowAndroid()
{
}

