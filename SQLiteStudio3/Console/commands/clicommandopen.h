#ifndef CLICOMMANDOPEN_H
#define CLICOMMANDOPEN_H

#include "clicommand.h"

class CliCommandOpen : public CliCommand
{
    public:
        static CliCommandOpen* create();

        void execute(QStringList args);
        bool validate(QStringList args);
};

#endif // CLICOMMANDOPEN_H
