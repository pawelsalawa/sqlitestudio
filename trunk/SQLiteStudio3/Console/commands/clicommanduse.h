#ifndef CLICOMMANDUSE_H
#define CLICOMMANDUSE_H

#include "clicommand.h"

class CliCommandUse : public CliCommand
{
    public:
        void execute();
        QString shortHelp() const;
        QString fullHelp() const;
        void defineSyntax();
};

#endif // CLICOMMANDUSE_H
