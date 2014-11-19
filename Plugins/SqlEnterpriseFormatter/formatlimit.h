#ifndef FORMATLIMIT_H
#define FORMATLIMIT_H

#include "formatstatement.h"

class SqliteLimit;

class FormatLimit : public FormatStatement
{
    public:
        FormatLimit(SqliteLimit* limit);

        void formatInternal();

    private:
        SqliteLimit *limit = nullptr;
};

#endif // FORMATLIMIT_H
