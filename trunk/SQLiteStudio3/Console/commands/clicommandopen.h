#ifndef CLICOMMANDOPEN_H
#define CLICOMMANDOPEN_H

#include "clicommand.h"

class CliCommandOpen : public CliCommand
{
    public:
        bool execute(QStringList args);
        bool validate(QStringList args);
};

#endif // CLICOMMANDOPEN_H
