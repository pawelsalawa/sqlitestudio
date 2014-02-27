#ifndef CLICOMMANDEXIT_H
#define CLICOMMANDEXIT_H

#include "clicommand.h"

class CliCommandExit : public CliCommand
{
    public:
        bool execute(QStringList args);
        bool validate(QStringList args);
        QString shortHelp() const;
        QString fullHelp() const;
        QString usage() const;
};

#endif // CLICOMMANDEXIT_H
