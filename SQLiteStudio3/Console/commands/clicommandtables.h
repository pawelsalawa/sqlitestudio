#ifndef CLICOMMANDTABLES_H
#define CLICOMMANDTABLES_H

#include "clicommand.h"

class CliCommandTables : public CliCommand
{
    public:
        void execute();
        QString shortHelp() const;
        QString fullHelp() const;
        void defineSyntax();

    private:
        enum ArgIds
        {
            SHOW_SYSTEM_TABLES
        };
};

#endif // CLICOMMANDTABLES_H
