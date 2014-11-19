#ifndef FORMATCOPY_H
#define FORMATCOPY_H

#include "formatstatement.h"

class SqliteCopy;

class FormatCopy : public FormatStatement
{
    public:
        FormatCopy(SqliteCopy* copy);

    protected:
        void formatInternal();

    private:
        SqliteCopy* copy = nullptr;
};

#endif // FORMATCOPY_H
