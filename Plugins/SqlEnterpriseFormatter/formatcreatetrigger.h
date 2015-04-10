#ifndef FORMATCREATETRIGGER_H
#define FORMATCREATETRIGGER_H

#include "formatstatement.h"
#include "parser/ast/sqlitecreatetrigger.h"
#include "common/global.h"

class FormatCreateTrigger : public FormatStatement
{
    public:
        FormatCreateTrigger(SqliteCreateTrigger* createTrig);

    protected:
        void formatInternal();

    private:
        SqliteCreateTrigger* createTrig = nullptr;

        static_char* TRIGGER_MARK = "TRIGGER";
};

class FormatCreateTriggerEvent : public FormatStatement
{
    public:
        FormatCreateTriggerEvent(SqliteCreateTrigger::Event* ev);
        void setLineUpKeyword(const QString& lineUpKw);

    protected:
        void formatInternal();

    private:
        SqliteCreateTrigger::Event* ev = nullptr;
        QString lineUpKw;

        static_char* TRIGGER_MARK = "TRIGGER";
};

#endif // FORMATCREATETRIGGER_H
