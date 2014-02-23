#ifndef CLICOMMANDREMOVE_H
#define CLICOMMANDREMOVE_H

#include "clicommand.h"

class CliCommandRemove : public CliCommand
{
    public:
        static CliCommandRemove* create();

        void execute(QStringList args);
        bool validate(QStringList args);
};

#endif // CLICOMMANDREMOVE_H
