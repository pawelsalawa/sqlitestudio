#ifndef FORMATCREATETRIGGER_H
#define FORMATCREATETRIGGER_H

#include "formatstatement.h"
#include "parser/ast/sqlitecreatetrigger.h"

class FormatCreateTrigger : public FormatStatement
{
    public:
        FormatCreateTrigger(SqliteCreateTrigger* createTrig);

    protected:
        void formatInternal();

    private:
        SqliteCreateTrigger* createTrig;
};

class FormatCreateTriggerEvent : public FormatStatement
{
    public:
        FormatCreateTriggerEvent(SqliteCreateTrigger::Event* ev);

    protected:
        void formatInternal();

    private:
        SqliteCreateTrigger::Event* ev;
};

#endif // FORMATCREATETRIGGER_H
