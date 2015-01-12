#ifndef CLICOMMANDCD_H
#define CLICOMMANDCD_H

#include "clicommand.h"

class CliCommandCd : public CliCommand
{
        Q_OBJECT

    public:
        void execute();
        QString shortHelp() const;
        QString fullHelp() const;
        void defineSyntax();
};

#endif // CLICOMMANDCD_H
