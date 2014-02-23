#ifndef CODEFORMATTER_H
#define CODEFORMATTER_H

#include "coreSQLiteStudio_global.h"
#include "parser/ast/sqlitequery.h"

class SqlFormatterPlugin;

class API_EXPORT SqlFormatter
{
    public:
        QString format(const QString& sql, Dialect dialect);
        QString format(SqliteQueryPtr query);

        void setFormatter(SqlFormatterPlugin* formatterPlugin);
        SqlFormatterPlugin* getFormatter();

    private:
        SqlFormatterPlugin* currentFormatter = nullptr;
};

#endif // CODEFORMATTER_H
