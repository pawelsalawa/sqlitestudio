#ifndef FORMATUPDATE_H
#define FORMATUPDATE_H

#include "formatstatement.h"

class SqliteUpdate;

class FormatUpdate : public FormatStatement
{
    public:
        FormatUpdate(SqliteUpdate* upd);

    protected:
        void formatInternal();

    private:
        SqliteUpdate* upd;
};

#endif // FORMATUPDATE_H
