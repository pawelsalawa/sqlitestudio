#ifndef FORMATVACUUM_H
#define FORMATVACUUM_H

#include "formatstatement.h"

class SqliteVacuum;

class FormatVacuum : public FormatStatement
{
    public:
        FormatVacuum(SqliteVacuum* vacuum);

    protected:
        void formatInternal();

    private:
        SqliteVacuum* vacuum;
};

#endif // FORMATVACUUM_H
