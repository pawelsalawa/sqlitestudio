#ifndef CLICOMMANDNULLVALUE_H
#define CLICOMMANDNULLVALUE_H

#include "clicommand.h"

class CliCommandNullValue : public CliCommand
{
    public:
        bool execute(QStringList args);
        bool validate(QStringList args);
        QString shortHelp() const;
        QString fullHelp() const;
        QString usage() const;
        QStringList aliases() const;
};

#endif // CLICOMMANDNULLVALUE_H
