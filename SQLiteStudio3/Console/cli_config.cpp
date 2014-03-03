#include "cli_config.h"

CFG_DEFINE(Cli)

CliResultsDisplay::Mode CliResultsDisplay::mode(const QString& mode)
{
    if (mode == "ROW")
        return ROW;

    if (mode == "FIXED")
        return FIXED;

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
    }
    return "CLASSIC";
}


void CliResultsDisplay::staticInit()
{
    qRegisterMetaType<CliResultsDisplay::Mode>();
    qRegisterMetaTypeStreamOperators<CliResultsDisplay::Mode>();
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
