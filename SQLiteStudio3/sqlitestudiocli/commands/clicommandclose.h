#ifndef CLICOMMANDCLOSE_H
#define CLICOMMANDCLOSE_H

#include "clicommand.h"

class CliCommandClose : public CliCommand
{
        Q_OBJECT

    public:
        void execute();
        QString shortHelp() const;
        QString fullHelp() const;
        void defineSyntax();
};

#endif // CLICOMMANDCLOSE_H
