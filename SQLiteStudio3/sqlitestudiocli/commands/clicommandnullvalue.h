#ifndef CLICOMMANDNULLVALUE_H
#define CLICOMMANDNULLVALUE_H

#include "clicommand.h"

class CliCommandNullValue : public CliCommand
{
        Q_OBJECT

    public:
        void execute();
        QString shortHelp() const;
        QString fullHelp() const;
        void defineSyntax();
};

#endif // CLICOMMANDNULLVALUE_H
