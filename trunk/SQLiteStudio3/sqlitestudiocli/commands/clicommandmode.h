#ifndef CLICOMMANDMODE_H
#define CLICOMMANDMODE_H

#include "clicommand.h"

class CliCommandMode : public CliCommand
{
        Q_OBJECT

    public:
        void execute();
        QString shortHelp() const;
        QString fullHelp() const;
        void defineSyntax();

    private:
        enum ArgIgs
        {
            MODE
        };
};

#endif // CLICOMMANDMODE_H
