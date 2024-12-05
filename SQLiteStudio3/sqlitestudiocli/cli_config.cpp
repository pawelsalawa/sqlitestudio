#include "cli_config.h"
#include <QDataStream>

CFG_DEFINE(Cli)

CliResultsDisplay::Mode CliResultsDisplay::mode(const QString& mode)
{
    if (mode == "ROW")
        return ROW;

    if (mode == "FIXED")
        return FIXED;

    if (mode == "COLUMNS")
        return COLUMNS;

    return CLASSIC;
}

QString CliResultsDisplay::mode(CliResultsDisplay::Mode mode)
{
    switch (mode)
    {
        case ROW:
            return "ROW";
        case FIXED:
            return "FIXED";
        case CLASSIC:
            return "CLASSIC";
        case COLUMNS:
            return "COLUMNS";
    }
    return "CLASSIC";
}


void CliResultsDisplay::staticInit()
{
    qRegisterMetaType<CliResultsDisplay::Mode>();
#if QT_VERSION < 0x060000
    qRegisterMetaTypeStreamOperators<CliResultsDisplay::Mode>();
#else
    // Qt 6 does it automatically
#endif
}


QDataStream& operator<<(QDataStream& out, const CliResultsDisplay::Mode& mode)
{
    out << static_cast<int>(mode);
    return out;
}

QDataStream& operator>>(QDataStream& in, CliResultsDisplay::Mode& mode)
{
    int modeEnum;
    in >> modeEnum;
    mode = static_cast<CliResultsDisplay::Mode>(modeEnum);
    return in;
}
