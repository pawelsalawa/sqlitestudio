#include "sqlenterpriseformatter.h"

CFG_DEFINE(SqlEnterpriseFormatterConfig)

SqlEnterpriseFormatter::SqlEnterpriseFormatter()
{
}

QString SqlEnterpriseFormatter::format(SqliteQueryPtr query)
{
    return query->detokenize();
}

bool SqlEnterpriseFormatter::init()
{
    Q_INIT_RESOURCE(sqlenterpriseformatter);
    return GenericPlugin::init();
}

void SqlEnterpriseFormatter::deinit()
{
    Q_CLEANUP_RESOURCE(sqlenterpriseformatter);
}

QString Cfg::getNameWrapperStr(NameWrapper wrapper)
{
    return wrapObjName(QObject::tr("name", "example name wrapper"), wrapper);
}
