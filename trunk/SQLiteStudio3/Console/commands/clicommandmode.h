#ifndef CLICOMMANDMODE_H
#define CLICOMMANDMODE_H

#include "clicommand.h"

class CliCommandMode : public CliCommand
{
    public:
        bool execute(QStringList args);
        bool validate(QStringList args);
        QString shortHelp() const;
        QString fullHelp() const;
        QString usage() const;
};

#endif // CLICOMMANDMODE_H
