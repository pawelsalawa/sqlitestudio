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

QStringList Cfg::getNameWrapperStrings()
{
    QStringList strings;
    for (NameWrapper nw : {NameWrapper::BRACKET, NameWrapper::DOUBLE_QUOTE, NameWrapper::BACK_QUOTE, NameWrapper::QUOTE})
        strings << wrapObjName(QObject::tr("name", "example name wrapper"), nw);

    return strings;
}
