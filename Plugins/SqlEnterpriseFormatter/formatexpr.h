#ifndef FORMATEXPR_H
#define FORMATEXPR_H

#include "formatstatement.h"

class SqliteExpr;

class FormatExpr : public FormatStatement
{
    public:
        FormatExpr(SqliteExpr* expr);

    protected:
        void formatInternal();

    private:
        SqliteExpr* expr;
};

#endif // FORMATEXPR_H
