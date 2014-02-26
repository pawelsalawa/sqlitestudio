#ifndef CLICOMMANDEXIT_H
#define CLICOMMANDEXIT_H

#include "clicommand.h"

class CliCommandExit : public CliCommand
{
    public:
        bool execute(QStringList args);
        bool validate(QStringList args);
};

#endif // CLICOMMANDEXIT_H
