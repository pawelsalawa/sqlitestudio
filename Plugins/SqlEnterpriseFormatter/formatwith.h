#ifndef FORMATWITH_H
#define FORMATWITH_H

#include "formatstatement.h"
#include "parser/ast/sqlitewith.h"

class FormatWith : public FormatStatement
{
    public:
        FormatWith(SqliteWith* with);

        void formatInternal();

    private:
        SqliteWith *with;
};

class FormatWithCommonTableExpression : public FormatStatement
{
    public:
        FormatWithCommonTableExpression(SqliteWith::CommonTableExpression* cte);

        void formatInternal();

    private:
        SqliteWith::CommonTableExpression* cte;
};

#endif // FORMATWITH_H
