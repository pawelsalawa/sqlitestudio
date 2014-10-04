#ifndef SQLFORMATTERSIMPLEPLUGIN_H
#define SQLFORMATTERSIMPLEPLUGIN_H

#include "sqlformattersimple_global.h"
#include "plugins/sqlformatterplugin.h"
#include "config_builder.h"
#include "plugins/genericplugin.h"
#include "plugins/uiconfiguredplugin.h"
#include <QObject>

CFG_CATEGORIES(SqlFormatterSimpleConfig,
     CFG_CATEGORY(SqlFormatterSimple,
         CFG_ENTRY(bool, UpperCaseKeywords, true)
         CFG_ENTRY(bool, TrimLongSpaces,    true)
     )
)

class SQLFORMATTERSIMPLESHARED_EXPORT SqlFormatterSimplePlugin : public GenericPlugin, public SqlFormatterPlugin, public UiConfiguredPlugin
{
        Q_OBJECT
        SQLITESTUDIO_PLUGIN("sqlformattersimple.json")

    public:
        SqlFormatterSimplePlugin();

        QString format(SqliteQueryPtr query);
        bool init();
        void deinit();
        QString getConfigUiForm() const;
        CfgMain* getMainUiConfig();
        void configDialogOpen();
        void configDialogClosed();

    private:
        CFG_LOCAL_PERSISTABLE(SqlFormatterSimpleConfig, cfg)
};

#endif // SQLFORMATTERSIMPLEPLUGIN_H
