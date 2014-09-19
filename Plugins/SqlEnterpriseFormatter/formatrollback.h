#ifndef FORMATROLLBACK_H
#define FORMATROLLBACK_H

#include "formatstatement.h"

class SqliteRollback;

class FormatRollback : public FormatStatement
{
    public:
        FormatRollback(SqliteRollback* rollback);

    protected:
        void formatInternal();

    private:
        SqliteRollback* rollback;
};

#endif // FORMATROLLBACK_H
