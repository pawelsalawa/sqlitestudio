#ifndef SQLENTERPRISEFORMATTER_H
#define SQLENTERPRISEFORMATTER_H

#include "sqlenterpriseformatter_global.h"
#include "plugins/genericplugin.h"
#include "plugins/sqlformatterplugin.h"

class SQLENTERPRISEFORMATTERSHARED_EXPORT SqlEnterpriseFormatter : public GenericPlugin, public SqlFormatterPlugin
{
    Q_OBJECT
    SQLITESTUDIO_PLUGIN("sqlenterpriseformatter.json")

public:
    SqlEnterpriseFormatter();

    QString format(SqliteQueryPtr query);
    bool init();
    void deinit();
};

#endif // SQLENTERPRISEFORMATTER_H
