#ifndef CLI_CONFIG_H
#define CLI_CONFIG_H

#include "cfginternals.h"

namespace CliResultsDisplay
{
    enum Mode
    {
        CLASSIC = 0,
        FIXED = 1,
        ROW = 2,
        COLUMNS = 3
    };

    Mode mode(const QString& mode);
    QString mode(Mode mode);
    void staticInit();

}

QDataStream &operator<<(QDataStream &out, const CliResultsDisplay::Mode& mode);
QDataStream &operator>>(QDataStream &in, CliResultsDisplay::Mode& mode);

Q_DECLARE_METATYPE(CliResultsDisplay::Mode)

CFG_CATEGORIES(Cli,
    CFG_CATEGORY(Console,
        CFG_ENTRY(QString,                 DefaultDatabase,    QString())
        CFG_ENTRY(QString,                 CommandPrefixChar,  ".")
        CFG_ENTRY(CliResultsDisplay::Mode, ResultsDisplayMode, CliResultsDisplay::CLASSIC)
        CFG_ENTRY(QString,                 NullValue,          "")
    )
)

CFG_DECLARE(Cli)
#define CFG_CLI CFG_INSTANCE(Cli)

#endif // CLI_CONFIG_H
