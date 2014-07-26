#ifndef SQLFORMATTERPLUGIN_H
#define SQLFORMATTERPLUGIN_H

#include "coreSQLiteStudio_global.h"
#include "codeformatterplugin.h"
#include "parser/ast/sqlitequery.h"

class API_EXPORT SqlFormatterPlugin : public CodeFormatterPlugin
{
    public:
        QString format(const QString& code, Db* contextDb);
        QString getLanguage() const;
        virtual QString format(SqliteQueryPtr query) = 0;
};

#endif // SQLFORMATTERPLUGIN_H
