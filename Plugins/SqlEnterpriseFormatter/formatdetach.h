#ifndef FORMATDETACH_H
#define FORMATDETACH_H

#include "formatstatement.h"

class SqliteDetach;

class FormatDetach : public FormatStatement
{
    public:
        FormatDetach(SqliteDetach* detach);

    protected:
        void formatInternal();

    private:
        SqliteDetach* detach = nullptr;
};

#endif // FORMATDETACH_H
