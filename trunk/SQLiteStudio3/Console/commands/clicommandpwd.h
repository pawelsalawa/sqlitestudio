#ifndef CLICOMMANDPWD_H
#define CLICOMMANDPWD_H

#include "clicommand.h"

class CliCommandPwd : public CliCommand
{
    public:
        bool execute(QStringList args);
        bool validate(QStringList args);
        QString shortHelp() const;
        QString fullHelp() const;
        QString usage() const;
};

#endif // CLICOMMANDPWD_H
