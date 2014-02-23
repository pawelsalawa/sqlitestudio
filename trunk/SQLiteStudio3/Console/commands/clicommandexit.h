#ifndef CLICOMMANDEXIT_H
#define CLICOMMANDEXIT_H

#include "clicommand.h"

class CliCommandExit : public CliCommand
{
    public:
        static CliCommandExit* create();

        void execute(QStringList args);
        bool validate(QStringList args);
};

#endif // CLICOMMANDEXIT_H
