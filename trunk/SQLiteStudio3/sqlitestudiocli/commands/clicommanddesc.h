#ifndef CLICOMMANDDESC_H
#define CLICOMMANDDESC_H

#include "clicommand.h"

class CliCommandDesc : public CliCommand
{
        Q_OBJECT

    public:
        CliCommandDesc();
        void execute();
        QString shortHelp() const;
        QString fullHelp() const;
        void defineSyntax();
};

#endif // CLICOMMANDDESC_H
