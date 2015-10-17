#ifndef SQLRESULTROWANDROID_H
#define SQLRESULTROWANDROID_H

#include "db/sqlresultsrow.h"

class SqlResultRowAndroid : public SqlResultsRow
{
    public:
        SqlResultRowAndroid(const QVariantHash& resultMap, const QVariantList& resultList);
        ~SqlResultRowAndroid();
};

#endif // SQLRESULTROWANDROID_H
