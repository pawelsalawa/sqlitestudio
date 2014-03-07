#ifndef CLICOMMANDHELP_H
#define CLICOMMANDHELP_H

#include "clicommand.h"

class CliCommandHelp : public CliCommand
{
    public:
        void execute(const QStringList& args);
        QString shortHelp() const;
        QString fullHelp() const;
        void defineSyntax();

    private:
        void printHelp(const QString& cmd);
        void printHelp();
};

#endif // CLICOMMANDHELP_H
