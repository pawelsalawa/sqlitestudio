#ifndef FORMATRELEASE_H
#define FORMATRELEASE_H

#include "formatstatement.h"

class SqliteRelease;

class FormatRelease : public FormatStatement
{
    public:
        FormatRelease(SqliteRelease* release);

    protected:
        void formatInternal();

    private:
        SqliteRelease* release = nullptr;
};

#endif // FORMATRELEASE_H
