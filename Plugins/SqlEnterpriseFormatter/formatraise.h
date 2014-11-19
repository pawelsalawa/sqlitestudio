#ifndef FORMATRAISE_H
#define FORMATRAISE_H

#include "formatstatement.h"

class SqliteRaise;

class FormatRaise : public FormatStatement
{
    public:
        FormatRaise(SqliteRaise* raise);

    protected:
        void formatInternal();

    private:
        SqliteRaise* raise = nullptr;
};

#endif // FORMATRAISE_H
