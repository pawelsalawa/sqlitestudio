#ifndef CLICOMMANDADD_H
#define CLICOMMANDADD_H

#include "clicommand.h"

class CliCommandAdd : public CliCommand
{
    public:
        static CliCommandAdd* create();

        void execute(QStringList args);
        bool validate(QStringList args);
};

#endif // CLICOMMANDADD_H
