#ifndef CLICOMMANDCLOSE_H
#define CLICOMMANDCLOSE_H

#include "clicommand.h"

class CliCommandClose : public CliCommand
{
    public:
        bool execute(QStringList args);
        bool validate(QStringList args);
        QString shortHelp() const;
        QString fullHelp() const;
        QString usage() const;
};

#endif // CLICOMMANDCLOSE_H
