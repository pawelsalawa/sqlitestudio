#ifndef FORMATBEGINTRANS_H
#define FORMATBEGINTRANS_H

#include "formatstatement.h"

class SqliteBeginTrans;

class FormatBeginTrans : public FormatStatement
{
    public:
        FormatBeginTrans(SqliteBeginTrans* bt);

    protected:
        void formatInternal();

    private:
        SqliteBeginTrans* bt = nullptr;
};

#endif // FORMATBEGINTRANS_H
