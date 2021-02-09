#ifndef FORMATFILTEROVER_H
#define FORMATFILTEROVER_H

#include "formatstatement.h"
#include "parser/ast/sqlitefilterover.h"

class FormatFilterOver : public FormatStatement
{
    public:
        FormatFilterOver(SqliteFilterOver* filterOver);

    protected:
        void formatInternal();

    private:
        SqliteFilterOver* filterOver = nullptr;
};

class FormatFilterOverFilter : public FormatStatement
{
    public:
        FormatFilterOverFilter(SqliteFilterOver::Filter* filter);

    protected:
        void formatInternal();

    private:
        SqliteFilterOver::Filter* filter = nullptr;
};

class FormatFilterOverOver: public FormatStatement
{
    public:
        FormatFilterOverOver(SqliteFilterOver::Over* over);

    protected:
        void formatInternal();

    private:
        SqliteFilterOver::Over* over = nullptr;
};

#endif // FORMATFILTEROVER_H
