#ifndef CLICOMMANDCLOSE_H
#define CLICOMMANDCLOSE_H

#include "clicommand.h"

class CliCommandClose : public CliCommand
{
    public:
        bool execute(QStringList args);
        bool validate(QStringList args);
};

#endif // CLICOMMANDCLOSE_H
