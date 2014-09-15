#ifndef FORMATSELECT_H
#define FORMATSELECT_H

#include "formatstatement.h"
#include "parser/ast/sqliteselect.h"

class FormatSelect : public FormatStatement
{
    public:
        FormatSelect(SqliteSelect *select);

        QString format();

    private:
        SqliteSelect* select;
};

#endif // FORMATSELECT_H
