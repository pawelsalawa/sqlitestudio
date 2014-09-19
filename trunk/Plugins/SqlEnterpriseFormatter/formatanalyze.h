#ifndef FORMATANALYZE_H
#define FORMATANALYZE_H

#include "formatstatement.h"

class SqliteAnalyze;

class FormatAnalyze : public FormatStatement
{
    public:
        FormatAnalyze(SqliteAnalyze* analyze);

    protected:
        void formatInternal();

    private:
        SqliteAnalyze* analyze;
};

#endif // FORMATANALYZE_H
