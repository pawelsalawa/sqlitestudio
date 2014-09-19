#ifndef FORMATDROPTABLE_H
#define FORMATDROPTABLE_H

#include "formatstatement.h"

class SqliteDropTable;

class FormatDropTable : public FormatStatement
{
    public:
        FormatDropTable(SqliteDropTable* dropTable);

    protected:
        void formatInternal();

    private:
        SqliteDropTable* dropTable;
};

#endif // FORMATDROPTABLE_H
