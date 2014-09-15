#include "sqlenterpriseformatter.h"
#include "formatstatement.h"
#include "common/unused.h"
#include <QDebug>

CFG_DEFINE(SqlEnterpriseFormatterConfig)

SqlEnterpriseFormatter::SqlEnterpriseFormatter()
{
}

QString SqlEnterpriseFormatter::format(SqliteQueryPtr query)
{
    FormatStatement *formatStmt = FormatStatement::forQuery(query.data());
    if (!formatStmt)
        return query->detokenize();

    QString formatted = formatStmt->format();
    delete formatStmt;

    return formatted;
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

void SqlEnterpriseFormatter::updatePreview()
{
    qDebug() << "update preview";
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

void SqlEnterpriseFormatter::configModified(CfgEntry* key, const QVariant& value)
{
    UNUSED(value);
    if (key->getMain() != CFG_ADV_FMT)
        return;

    updatePreview();
}
