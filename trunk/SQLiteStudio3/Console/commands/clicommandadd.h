#ifndef CLICOMMANDADD_H
#define CLICOMMANDADD_H

#include "clicommand.h"

class CliCommandAdd : public CliCommand
{
    public:
        bool execute(QStringList args);
        bool validate(QStringList args);
        QString shortHelp() const;
        QString fullHelp() const;
        QString usage() const;
};

#endif // CLICOMMANDADD_H
