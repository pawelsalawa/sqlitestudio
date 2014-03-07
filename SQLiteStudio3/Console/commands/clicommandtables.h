#ifndef CLICOMMANDTABLES_H
#define CLICOMMANDTABLES_H

#include "clicommand.h"

class CliCommandTables : public CliCommand
{
    public:
        void execute(const QStringList& args);
        QString shortHelp() const;
        QString fullHelp() const;
        void defineSyntax();
};

#endif // CLICOMMANDTABLES_H
