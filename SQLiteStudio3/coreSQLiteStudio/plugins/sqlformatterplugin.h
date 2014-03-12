#ifndef SQLFORMATTERPLUGIN_H
#define SQLFORMATTERPLUGIN_H

#include "coreSQLiteStudio_global.h"
#include "plugin.h"
#include "parser/ast/sqlitequery.h"

class API_EXPORT SqlFormatterPlugin : virtual public Plugin
{
    public:
        virtual QString format(SqliteQueryPtr query) = 0;
};

#endif // SQLFORMATTERPLUGIN_H
