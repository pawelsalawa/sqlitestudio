#ifndef SQLENTERPRISEFORMATTER_H
#define SQLENTERPRISEFORMATTER_H

#include "sqlenterpriseformatter_global.h"
#include "plugins/genericplugin.h"
#include "plugins/sqlformatterplugin.h"
#include "config_builder.h"
#include "common/utils_sql.h"

namespace Cfg
{
    QString getNameWrapperStr(NameWrapper wrapper);
    QStringList getNameWrapperStrings();
}

CFG_CATEGORIES(SqlEnterpriseFormatterConfig,
    CFG_CATEGORY(SqlEnterpriseFormatter,
        CFG_ENTRY(int,         TabSize,                   4)
        CFG_ENTRY(bool,        LineUpKeywords,            true)
        CFG_ENTRY(bool,        IndentParenthesisBlock,    true)
        CFG_ENTRY(bool,        NlBeforeOpenParDef,        false)
        CFG_ENTRY(bool,        NlAfterOpenParDef,         true)
        CFG_ENTRY(bool,        NlBeforeCloseParDef,       true)
        CFG_ENTRY(bool,        NlAfterCloseParDef,        true)
        CFG_ENTRY(bool,        NlBeforeOpenParExpr,       false)
        CFG_ENTRY(bool,        NlAfterOpenParExpr,        false)
        CFG_ENTRY(bool,        NlBeforeCloseParExpr,      false)
        CFG_ENTRY(bool,        NlAfterCloseParExpr,       false)
        CFG_ENTRY(bool,        NlAfterComma,              true)
        CFG_ENTRY(bool,        NlAfterCommaInFnArg,       false)
        CFG_ENTRY(bool,        NlAfterSemicolon,          true)
        CFG_ENTRY(bool,        NlNeverBeforeSemicolon,    true)
        CFG_ENTRY(bool,        SpaceBeforeCommaInList,    false)
        CFG_ENTRY(bool,        SpaceAfterCommaInList,     true)
        CFG_ENTRY(bool,        SpaceBeforeOpenPar,        true)
        CFG_ENTRY(bool,        SpaceAfterOpenPar,         true)
        CFG_ENTRY(bool,        SpaceBeforeClosePar,       true)
        CFG_ENTRY(bool,        SpaceAfterClosePar,        true)
        CFG_ENTRY(bool,        SpaceBeforeDot,            false)
        CFG_ENTRY(bool,        SpaceAfterDot,             false)
        CFG_ENTRY(bool,        SpaceBeforeMathOp,         true)
        CFG_ENTRY(bool,        SpaceAfterMathOp,          true)
        CFG_ENTRY(bool,        SpaceNeverBeforeSemicolon, true)
        CFG_ENTRY(bool,        UppercaseKeywords,         true)
        CFG_ENTRY(bool,        UppercaseDataTypes,        true)
        CFG_ENTRY(bool,        AlwaysUseNameWrapping,     false)
        CFG_ENTRY(QString,     PrefferedWrapper,          getNameWrapperStr(NameWrapper::BRACKET))
        CFG_ENTRY(QStringList, Wrappers,                  getNameWrapperStrings())
    )
)

#define CFG_ADV_FMT CFG_INSTANCE(SqlEnterpriseFormatterConfig)

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
