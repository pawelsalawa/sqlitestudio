#ifndef CLICOMMANDHELP_H
#define CLICOMMANDHELP_H

#include "clicommand.h"

class CliCommandHelp : public CliCommand
{
    public:
        bool execute(QStringList args);
        bool validate(QStringList args);
        QString shortHelp() const;
        QString fullHelp() const;
        QString usage() const;

    private:
        void printHelp(const QString& cmd);
        void printHelp();
};

#endif // CLICOMMANDHELP_H
