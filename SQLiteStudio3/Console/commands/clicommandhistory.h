#ifndef CLICOMMANDHISTORY_H
#define CLICOMMANDHISTORY_H

#include "clicommand.h"

class CliCommandHistory : public CliCommand
{
    public:
        bool execute(QStringList args);
        bool validate(QStringList args);
        QString shortHelp() const;
        QString fullHelp() const;
        QString usage() const;
};

#endif // CLICOMMANDHISTORY_H
