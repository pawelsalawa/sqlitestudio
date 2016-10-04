#ifndef FORMATEXPR_H
#define FORMATEXPR_H

#include <QRegularExpression>
#include "formatstatement.h"


class SqliteExpr;

class FormatExpr : public FormatStatement
{
    public:
        FormatExpr(SqliteExpr* expr);

    protected:
        void formatInternal();

    private:
        static QRegularExpression WORD_ONLY_RE;

        SqliteExpr* expr = nullptr;
};

#endif // FORMATEXPR_H
