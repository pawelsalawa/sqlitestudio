#ifndef SQLFORMATTERSIMPLEPLUGIN_H
#define SQLFORMATTERSIMPLEPLUGIN_H

#include "sqlformattersimple_global.h"
#include "plugins/sqlformatterplugin.h"
#include "config_builder.h"
#include "plugins/genericplugin.h"
#include <QObject>

CFG_CATEGORIES(SqlFormatterSimpleConfig,
     CFG_CATEGORY(SqlFormatterSimple,
         CFG_ENTRY(bool, UpperCaseKeywords, true)
         CFG_ENTRY(bool, TrimLongSpaces,    true)
     )
)

#define CFG_SIMPLE_FMT CFG_INSTANCE(SqlFormatterSimpleConfig)

class SQLFORMATTERSIMPLESHARED_EXPORT SqlFormatterSimplePlugin : public GenericPlugin, public SqlFormatterPlugin
{
        Q_OBJECT
        SQLITESTUDIO_PLUGIN("sqlformattersimple.json")

    public:
        SqlFormatterSimplePlugin();

        QString format(SqliteQueryPtr query);
        bool init();
        void deinit();
};

#endif // SQLFORMATTERSIMPLEPLUGIN_H
