#ifndef FORMATFOREIGNKEY_H
#define FORMATFOREIGNKEY_H

#include "formatstatement.h"
#include "parser/ast/sqliteforeignkey.h"

class FormatForeignKey : public FormatStatement
{
    public:
        FormatForeignKey(SqliteForeignKey* fk);

    protected:
        void formatInternal();

    private:
        SqliteForeignKey* fk = nullptr;
};

class FormatForeignKeyCondition : public FormatStatement
{
    public:
        FormatForeignKeyCondition(SqliteForeignKey::Condition* cond);

    protected:
        void formatInternal();

    private:
        void formatReaction();

        SqliteForeignKey::Condition* cond = nullptr;
};

#endif // FORMATFOREIGNKEY_H
