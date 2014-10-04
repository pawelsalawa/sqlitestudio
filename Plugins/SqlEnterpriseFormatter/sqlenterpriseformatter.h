#ifndef SQLENTERPRISEFORMATTER_H
#define SQLENTERPRISEFORMATTER_H

#include "sqlenterpriseformatter_global.h"
#include "plugins/genericplugin.h"
#include "plugins/sqlformatterplugin.h"
#include "config_builder.h"
#include "common/utils_sql.h"
#include "plugins/uiconfiguredplugin.h"
#include "parser/ast/sqlitequery.h"

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
        CFG_ENTRY(bool,        NlAfterCommaInExpr,        false)
        CFG_ENTRY(bool,        NlAfterSemicolon,          true)
        CFG_ENTRY(bool,        NlNeverBeforeSemicolon,    true)
        CFG_ENTRY(bool,        NlBetweenConstraints,      true)
        CFG_ENTRY(bool,        SpaceBeforeCommaInList,    false)
        CFG_ENTRY(bool,        SpaceAfterCommaInList,     true)
        CFG_ENTRY(bool,        SpaceBeforeOpenPar,        true)
        CFG_ENTRY(bool,        SpaceAfterOpenPar,         false)
        CFG_ENTRY(bool,        SpaceBeforeClosePar,       false)
        CFG_ENTRY(bool,        SpaceAfterClosePar,        true)
        CFG_ENTRY(bool,        SpaceBeforeDot,            false)
        CFG_ENTRY(bool,        SpaceAfterDot,             false)
        CFG_ENTRY(bool,        SpaceBeforeMathOp,         true)
        CFG_ENTRY(bool,        SpaceAfterMathOp,          true)
        CFG_ENTRY(bool,        NoSpaceAfterFunctionName,  true)
        CFG_ENTRY(bool,        SpaceNeverBeforeSemicolon, true)
        CFG_ENTRY(bool,        SpaceNeverBeforeComma,     true)
        CFG_ENTRY(bool,        UppercaseKeywords,         true)
        CFG_ENTRY(bool,        UppercaseDataTypes,        true)
        CFG_ENTRY(bool,        AlwaysUseNameWrapping,     false)
        CFG_ENTRY(QString,     PrefferedWrapper,          getNameWrapperStr(NameWrapper::BRACKET))
        CFG_ENTRY(QStringList, Wrappers,                  getNameWrapperStrings(),                  false)
        CFG_ENTRY(QString,     PreviewCode,               QString(),                                false)
    )
)

class SQLENTERPRISEFORMATTERSHARED_EXPORT SqlEnterpriseFormatter : public GenericPlugin, public SqlFormatterPlugin, public UiConfiguredPlugin
{
    Q_OBJECT
    SQLITESTUDIO_PLUGIN("sqlenterpriseformatter.json")

    public:
        SqlEnterpriseFormatter();

        QString format(SqliteQueryPtr query);
        bool init();
        void deinit();
        CfgMain* getMainUiConfig();
        QString getConfigUiForm() const;
        void configDialogOpen();
        void configDialogClosed();

    private:
        QList<SqliteQueryPtr> previewQueries;
        CFG_LOCAL_PERSISTABLE(SqlEnterpriseFormatterConfig, cfg)

    private slots:
        void updatePreview();
        void configModified(CfgEntry* entry);
};

#endif // SQLENTERPRISEFORMATTER_H
