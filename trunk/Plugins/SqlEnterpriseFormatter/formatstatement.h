#ifndef FORMATSTATEMENT_H
#define FORMATSTATEMENT_H

#include "parser/ast/sqlitequery.h"
#include <QString>

class FormatStatement
{
    public:
        FormatStatement();

        virtual QString format() = 0;

        static FormatStatement* forQuery(SqliteQuery *query);
};

#endif // FORMATSTATEMENT_H
