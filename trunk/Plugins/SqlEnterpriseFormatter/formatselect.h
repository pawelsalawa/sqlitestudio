#ifndef FORMATSELECT_H
#define FORMATSELECT_H

#include "formatstatement.h"
#include "parser/ast/sqliteselect.h"

class FormatSelect : public FormatStatement
{
    public:
        FormatSelect(SqliteSelect *select);

    protected:
        void formatInternal();
        void resetInternal();

    private:
        SqliteSelect* select;
};

class FormatSelectCore : public FormatStatement
{
    public:
        FormatSelectCore(SqliteSelect::Core *core);

    protected:
        void formatInternal();
        void resetInternal();

    private:
        SqliteSelect::Core* core;
};

class FormatSelectCoreResultColumn : public FormatStatement
{
    public:
        FormatSelectCoreResultColumn(SqliteSelect::Core::ResultColumn *resCol);

    protected:
        void formatInternal();
        void resetInternal();

    private:
        SqliteSelect::Core::ResultColumn *resCol;
};

#endif // FORMATSELECT_H
