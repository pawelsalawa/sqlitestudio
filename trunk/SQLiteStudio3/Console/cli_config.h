#ifndef CLI_CONFIG_H
#define CLI_CONFIG_H

#include "cfginternals.h"

namespace CliResultsDisplay
{
    enum Mode
    {
        CLASSIC = 0,
        FIXED = 1,
        ROW_BY_ROW = 2
    };
}

CFG_CATEGORIES(Cli,
    CFG_CATEGORY(Console,
        CFG_ENTRY(QString,                 DefaultDatabase,    QString())
        CFG_ENTRY(QString,                 CommandPrefixChar,  ".")
        CFG_ENTRY(int,                     ColumnMaxWidth,     20)
        CFG_ENTRY(CliResultsDisplay::Mode, ResultsDisplayMode, CliResultsDisplay::CLASSIC)
    )
)

CFG_DECLARE(Cli)
#define CFG_CLI CFG_INSTANCE(Cli)

#endif // CLI_CONFIG_H
