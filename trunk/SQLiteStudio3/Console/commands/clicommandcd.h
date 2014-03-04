#ifndef CLICOMMANDCD_H
#define CLICOMMANDCD_H

#include "clicommand.h"

class CliCommandCd : public CliCommand
{
    public:
        bool execute(QStringList args);
        bool validate(QStringList args);
        QString shortHelp() const;
        QString fullHelp() const;
        QString usage() const;
};

#endif // CLICOMMANDCD_H
