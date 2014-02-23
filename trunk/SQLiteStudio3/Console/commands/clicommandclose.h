#ifndef CLICOMMANDCLOSE_H
#define CLICOMMANDCLOSE_H

#include "clicommand.h"

class CliCommandClose : public CliCommand
{
    public:
        static CliCommandClose* create();

        void execute(QStringList args);
        bool validate(QStringList args);
};

#endif // CLICOMMANDCLOSE_H
