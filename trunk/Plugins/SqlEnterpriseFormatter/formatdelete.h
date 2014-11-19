#ifndef FORMATDELETE_H
#define FORMATDELETE_H

#include "formatstatement.h"

class SqliteDelete;

class FormatDelete : public FormatStatement
{
    public:
        FormatDelete(SqliteDelete* del);

    protected:
        void formatInternal();

    private:
        SqliteDelete* del = nullptr;
};

#endif // FORMATDELETE_H
