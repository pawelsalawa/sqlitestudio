#ifndef CLICOMMANDHELP_H
#define CLICOMMANDHELP_H

#include "clicommand.h"

class CliCommandHelp : public CliCommand
{
        Q_OBJECT

    public:
        void execute();
        QString shortHelp() const;
        QString fullHelp() const;
        void defineSyntax();

    private:
        void printHelp(const QString& cmd);
        void printHelp();
};

#endif // CLICOMMANDHELP_H
