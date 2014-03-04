#ifndef CLICOMMANDDIR_H
#define CLICOMMANDDIR_H

#include "clicommand.h"

class CliCommandDir : public CliCommand
{
    public:
        bool execute(QStringList args);
        bool validate(QStringList args);
        QString shortHelp() const;
        QString fullHelp() const;
        QString usage() const;
        QStringList aliases() const;
};

#endif // CLICOMMANDDIR_H
