#ifndef CLICOMMANDPWD_H
#define CLICOMMANDPWD_H

#include "clicommand.h"

class CliCommandPwd : public CliCommand
{
    public:
        void execute();
        QString shortHelp() const;
        QString fullHelp() const;
        void defineSyntax();
};

#endif // CLICOMMANDPWD_H
