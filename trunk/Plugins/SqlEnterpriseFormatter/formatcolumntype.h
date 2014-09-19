#ifndef FORMATCOLUMNTYPE_H
#define FORMATCOLUMNTYPE_H

#include "formatstatement.h"

class SqliteColumnType;

class FormatColumnType : public FormatStatement
{
    public:
        FormatColumnType(SqliteColumnType* colType);

    protected:
        void formatInternal();

    private:
        SqliteColumnType* colType;
};

#endif // FORMATCOLUMNTYPE_H
