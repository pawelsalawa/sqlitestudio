#ifndef CLICOMMANDHISTORY_H
#define CLICOMMANDHISTORY_H

#include "clicommand.h"

class CliCommandHistory : public CliCommand
{
    public:
        void execute(const QStringList& args);
        QString shortHelp() const;
        QString fullHelp() const;
        void defineSyntax();
};

#endif // CLICOMMANDHISTORY_H
