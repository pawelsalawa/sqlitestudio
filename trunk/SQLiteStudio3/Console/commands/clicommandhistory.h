#ifndef CLICOMMANDHISTORY_H
#define CLICOMMANDHISTORY_H

#include "clicommand.h"

class CliCommandHistory : public CliCommand
{
    public:
        void execute();
        QString shortHelp() const;
        QString fullHelp() const;
        void defineSyntax();

    private:
        enum ArgIds
        {
            OPER_TYPE,
            HIST_LIMIT,
            SHOW_LIMIT
        };

        void clear();
        void setMax(const QString& arg);
};

#endif // CLICOMMANDHISTORY_H
