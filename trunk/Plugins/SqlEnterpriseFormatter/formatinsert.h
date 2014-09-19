#ifndef FORMATINSERT_H
#define FORMATINSERT_H

#include "formatstatement.h"

class SqliteInsert;

class FormatInsert : public FormatStatement
{
    public:
        FormatInsert(SqliteInsert* insert);

    protected:
        void formatInternal();

    private:
        SqliteInsert* insert;
};

#endif // FORMATINSERT_H
