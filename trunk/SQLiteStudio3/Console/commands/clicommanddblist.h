#ifndef CLICOMMANDDBLIST_H
#define CLICOMMANDDBLIST_H

#include "clicommand.h"

class CliCommandDbList : public CliCommand
{
    public:
        bool execute(QStringList args);
        bool validate(QStringList args);

};

#endif // CLICOMMANDDBLIST_H
