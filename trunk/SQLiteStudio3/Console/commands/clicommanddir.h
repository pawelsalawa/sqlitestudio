#ifndef CLICOMMANDDIR_H
#define CLICOMMANDDIR_H

#include "clicommand.h"

class CliCommandDir : public CliCommand
{
    public:
        void execute();
        QString shortHelp() const;
        QString fullHelp() const;
        void defineSyntax();
};

#endif // CLICOMMANDDIR_H
