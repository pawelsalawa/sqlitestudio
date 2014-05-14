#ifndef QUERYEXECUTORVALUESMODE_H
#define QUERYEXECUTORVALUESMODE_H

#include "queryexecutorstep.h"

class QueryExecutorValuesMode : public QueryExecutorStep
{
        Q_OBJECT

    public:
        bool exec();
};

#endif // QUERYEXECUTORVALUESMODE_H
