#ifndef CLICOMMANDTABLES_H
#define CLICOMMANDTABLES_H

#include "clicommand.h"

class CliCommandTables : public CliCommand
{
    public:
        bool execute(QStringList args);
        bool validate(QStringList args);
        QString shortHelp() const;
        QString fullHelp() const;
        QString usage() const;
};

#endif // CLICOMMANDTABLES_H
