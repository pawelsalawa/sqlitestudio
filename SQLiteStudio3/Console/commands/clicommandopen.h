#ifndef CLICOMMANDOPEN_H
#define CLICOMMANDOPEN_H

#include "clicommand.h"

class CliCommandOpen : public CliCommand
{
    public:
        bool execute(QStringList args);
        bool validate(QStringList args);
        QString shortHelp() const;
        QString fullHelp() const;
        QString usage() const;
};

#endif // CLICOMMANDOPEN_H
