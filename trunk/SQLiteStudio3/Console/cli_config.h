#ifndef CLI_CONFIG_H
#define CLI_CONFIG_H

#include "cfginternals.h"

CFG_CATEGORIES(Cli,
    CFG_CATEGORY(General,
        CFG_ENTRY(QString, DefaultDatabase, "CliDefaultDatabase", QString())
    )
)

CFG_DECLARE(Cli)
#define CLI_CFG CFG_INSTANCE(Cli)

#endif // CLI_CONFIG_H
