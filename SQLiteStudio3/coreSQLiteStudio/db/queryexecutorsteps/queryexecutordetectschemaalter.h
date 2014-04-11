#include "queryexecutorstep.h"

#ifndef QUERYEXECUTORDETECTSCHEMAALTER_H
#define QUERYEXECUTORDETECTSCHEMAALTER_H

class QueryExecutorDetectSchemaAlter : public QueryExecutorStep
{
        Q_OBJECT

    public:
        bool exec();
};

#endif // QUERYEXECUTORDETECTSCHEMAALTER_H
