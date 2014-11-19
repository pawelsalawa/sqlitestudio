#ifndef FORMATORDERBY_H
#define FORMATORDERBY_H

#include "formatstatement.h"

class SqliteOrderBy;

class FormatOrderBy : public FormatStatement
{
    public:
        FormatOrderBy(SqliteOrderBy *orderBy);

        void formatInternal();

    private:
        SqliteOrderBy *orderBy = nullptr;
};

#endif // FORMATORDERBY_H
