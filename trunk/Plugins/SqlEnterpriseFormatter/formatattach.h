#ifndef FORMATATTACH_H
#define FORMATATTACH_H

#include "formatstatement.h"

class SqliteAttach;

class FormatAttach : public FormatStatement
{
    public:
        FormatAttach(SqliteAttach* att);

    protected:
        void formatInternal();

    private:
        SqliteAttach* att;
};

#endif // FORMATATTACH_H
