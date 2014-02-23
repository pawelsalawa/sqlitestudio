#ifndef SQLFUNCTIONPLUGIN_H
#define SQLFUNCTIONPLUGIN_H

#include "coreSQLiteStudio_global.h"
#include "plugin.h"

class Db;

class API_EXPORT SqlFunctionPlugin : virtual public Plugin
{
    public:
        virtual QVariant evaluate(Db* db, const QString& function, const QString& code, const QList<QVariant>& args, bool& success) = 0;
        virtual QString getLanguageName() const = 0;
        virtual QByteArray getIconData() const = 0;
};

#endif // SQLFUNCTIONPLUGIN_H
