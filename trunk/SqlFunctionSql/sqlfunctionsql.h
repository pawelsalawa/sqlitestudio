#ifndef SQLFUNCTIONSQL_H
#define SQLFUNCTIONSQL_H

#include "sqlfunctionsql_global.h"
#include "genericplugin.h"
#include "sqlfunctionplugin.h"
#include <QObject>

class SQLFUNCTIONSQLSHARED_EXPORT SqlFunctionSql : public GenericPlugin, public SqlFunctionPlugin
{
        Q_OBJECT

        SQLITESTUDIO_PLUGIN
        SQLITESTUDIO_PLUGIN_TITLE("SQL functions")
        SQLITESTUDIO_PLUGIN_DESC("Supports custom query functions implemented with SQL.")
        SQLITESTUDIO_PLUGIN_VERSION(10000)
        SQLITESTUDIO_PLUGIN_AUTHOR("sqlitestudio.pl")

    public:
        QVariant evaluate(Db* db, const QString& function, const QString& code, const QList<QVariant>& args, bool& success);
        QString getLanguageName() const;
        QByteArray getIconData() const;
};

#endif // SQLFUNCTIONSQL_H
