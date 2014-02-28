#ifndef CLI_CONFIG_H
#define CLI_CONFIG_H

#include "cfginternals.h"

CFG_CATEGORIES(Cli,
    CFG_CATEGORY(Console,
        CFG_ENTRY(QString, DefaultDatabase,   QString())
        CFG_ENTRY(QString, CommandPrefixChar, ".")
        CFG_ENTRY(int,     ColumnMaxWidth,    20)
    )
)

CFG_DECLARE(Cli)
#define CFG_CLI CFG_INSTANCE(Cli)

#endif // CLI_CONFIG_H
