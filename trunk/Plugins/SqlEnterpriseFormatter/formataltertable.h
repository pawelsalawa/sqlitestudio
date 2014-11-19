#ifndef FORMATALTERTABLE_H
#define FORMATALTERTABLE_H

#include "formatstatement.h"

class SqliteAlterTable;

class FormatAlterTable : public FormatStatement
{
    public:
        FormatAlterTable(SqliteAlterTable* alterTable);

    protected:
        void formatInternal();

    private:
        SqliteAlterTable* alterTable = nullptr;
};

#endif // FORMATALTERTABLE_H
