#ifndef CODEFORMATTERPLUGIN_H
#define CODEFORMATTERPLUGIN_H

#include "coreSQLiteStudio_global.h"
#include "plugin.h"

class Db;

class API_EXPORT CodeFormatterPlugin : virtual public Plugin
{
    public:
        virtual QString getLanguage() const = 0;
        virtual QString format(const QString& code, Db* contextDb) = 0;
};

#endif // CODEFORMATTERPLUGIN_H
