#ifndef FORMATREINDEX_H
#define FORMATREINDEX_H

#include "formatstatement.h"

class SqliteReindex;

class FormatReindex : public FormatStatement
{
    public:
        FormatReindex(SqliteReindex* reindex);

    protected:
        void formatInternal();

    private:
        SqliteReindex* reindex;
};

#endif // FORMATREINDEX_H
