#ifndef FORMATEMPTY_H
#define FORMATEMPTY_H

#include "formatstatement.h"

class SqliteEmptyQuery;

class FormatEmpty : public FormatStatement
{
    public:
        FormatEmpty(SqliteEmptyQuery* eq);

    protected:
        void formatInternal();
};

#endif // FORMATEMPTY_H
