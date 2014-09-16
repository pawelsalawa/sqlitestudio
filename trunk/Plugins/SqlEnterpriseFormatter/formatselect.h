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

    private:
        SqliteSelect* select;
};

class FormatSelectCore : public FormatStatement
{
    public:
        FormatSelectCore(SqliteSelect::Core *core);

    protected:
        void formatInternal();

    private:
        SqliteSelect::Core* core;
};

class FormatSelectCoreResultColumn : public FormatStatement
{
    public:
        FormatSelectCoreResultColumn(SqliteSelect::Core::ResultColumn *resCol);

    protected:
        void formatInternal();

    private:
        SqliteSelect::Core::ResultColumn *resCol;
};

class FormatSelectCoreSingleSource : public FormatStatement
{
    public:
        FormatSelectCoreSingleSource(SqliteSelect::Core::SingleSource *singleSource);

    protected:
        void formatInternal();

    private:
        SqliteSelect::Core::SingleSource *singleSource;
};

class FormatSelectCoreJoinOp : public FormatStatement
{
    public:
        FormatSelectCoreJoinOp(SqliteSelect::Core::JoinOp *joinOp);

    protected:
        void formatInternal();

    private:
        SqliteSelect::Core::JoinOp *joinOp;
};

class FormatSelectCoreJoinConstraint : public FormatStatement
{
    public:
        FormatSelectCoreJoinConstraint(SqliteSelect::Core::JoinConstraint *joinConstr);

    protected:
        void formatInternal();

    private:
        SqliteSelect::Core::JoinConstraint *joinConstr;
};

class FormatSelectCoreJoinSourceOther : public FormatStatement
{
    public:
        FormatSelectCoreJoinSourceOther(SqliteSelect::Core::JoinSourceOther *joinSourceOther);

    protected:
        void formatInternal();

    private:
        SqliteSelect::Core::JoinSourceOther *joinSourceOther;
};

class FormatSelectCoreJoinSource : public FormatStatement
{
    public:
        FormatSelectCoreJoinSource(SqliteSelect::Core::JoinSource *joinSource);

    protected:
        void formatInternal();

    private:
        SqliteSelect::Core::JoinSource *joinSource;
};

#endif // FORMATSELECT_H
