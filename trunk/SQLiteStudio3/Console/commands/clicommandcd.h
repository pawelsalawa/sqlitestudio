#ifndef CLICOMMANDCD_H
#define CLICOMMANDCD_H

#include "clicommand.h"

class CliCommandCd : public CliCommand
{
    public:
        void execute(const QStringList& args);
        QString shortHelp() const;
        QString fullHelp() const;
        void defineSyntax();
};

#endif // CLICOMMANDCD_H
