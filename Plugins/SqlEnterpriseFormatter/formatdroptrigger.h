#ifndef FORMATDROPTRIGGER_H
#define FORMATDROPTRIGGER_H

#include "formatstatement.h"

class SqliteDropTrigger;

class FormatDropTrigger : public FormatStatement
{
    public:
        FormatDropTrigger(SqliteDropTrigger* dropTrig);

    protected:
        void formatInternal();

    private:
        SqliteDropTrigger* dropTrig;
};

#endif // FORMATDROPTRIGGER_H
