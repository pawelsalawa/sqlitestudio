#ifndef CLICOMMANDTREE_H
#define CLICOMMANDTREE_H

#include "clicommand.h"

class CliCommandTree : public CliCommand
{
    public:
        void execute(const QStringList& args);
        QString shortHelp() const;
        QString fullHelp() const;
        void defineSyntax();

    private:
        enum Opts
        {
            COLUMNS
        };
};

#endif // CLICOMMANDTREE_H
