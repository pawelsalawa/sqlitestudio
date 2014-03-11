#ifndef CLICOMMANDHISTORY_H
#define CLICOMMANDHISTORY_H

#include "clicommand.h"

class CliCommandHistory : public CliCommand
{
    public:
        void execute(const QStringList& args);
        QString shortHelp() const;
        QString fullHelp() const;
        void defineSyntax();

    private:
        enum ArgIds
        {
            OPER_TYPE
        };
};

#endif // CLICOMMANDHISTORY_H
