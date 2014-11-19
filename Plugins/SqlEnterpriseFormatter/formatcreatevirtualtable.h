#ifndef FORMATCREATEVIRTUALTABLE_H
#define FORMATCREATEVIRTUALTABLE_H

#include "formatstatement.h"
#include "parser/token.h"

class SqliteCreateVirtualTable;

class FormatCreateVirtualTable : public FormatStatement
{
    public:
        FormatCreateVirtualTable(SqliteCreateVirtualTable* cvt);

    protected:
        void formatInternal();

    private:
        void handleToken(const TokenPtr& token);

        SqliteCreateVirtualTable* cvt = nullptr;
};

#endif // FORMATCREATEVIRTUALTABLE_H
