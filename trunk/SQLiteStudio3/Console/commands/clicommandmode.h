#ifndef CLICOMMANDMODE_H
#define CLICOMMANDMODE_H

#include "clicommand.h"

class CliCommandMode : public CliCommand
{
    public:
        void execute(const QStringList& args);
        QString shortHelp() const;
        QString fullHelp() const;
        void defineSyntax();

    protected:
        QStringList getCompletionValuesFor(int id);

    private:
        enum ArgIgs
        {
            MODE
        };
};

#endif // CLICOMMANDMODE_H
