#ifndef FORMATPRAGMA_H
#define FORMATPRAGMA_H

#include "formatstatement.h"

class SqlitePragma;

class FormatPragma : public FormatStatement
{
    public:
        FormatPragma(SqlitePragma* pragma);

    protected:
        void formatInternal();

    private:
        SqlitePragma* pragma = nullptr;
};

#endif // FORMATPRAGMA_H
