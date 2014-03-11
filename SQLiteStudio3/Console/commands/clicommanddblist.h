#ifndef CLICOMMANDDBLIST_H
#define CLICOMMANDDBLIST_H

#include "clicommand.h"

class CliCommandDbList : public CliCommand
{
    public:
        void execute();
        QString shortHelp() const;
        QString fullHelp() const;
        void defineSyntax();
};

#endif // CLICOMMANDDBLIST_H
